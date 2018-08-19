/************************************************************************************

	AstroMenace
	Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
	Copyright (c) 2006-2018 Mikhail Kurinnoi, Viewizard


	AstroMenace is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AstroMenace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AstroMenace. If not, see <https://www.gnu.org/licenses/>.


	Website: https://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

// TODO codestyle should be fixed

// TODO translate comments

#include "../object3d.h"
#include "../space_ship/space_ship.h"
#include "../projectile/projectile.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

constexpr float RadToDeg = 180.0f / 3.14159f; // convert radian to degree

} // unnamed namespace

// FIXME should be fixed, don't allow global scope interaction for local variables
// направление движения камеры
extern sVECTOR3D GameCameraMovement;
// скорость движения камеры
float GameCameraGetSpeed();


/*
 * Find turret target and angle to aim target with prediction.
 */
// TODO no beam weapon support?
// TODO probably, we need revise all turret-related code in order to use relative angles
//      for turret rotation, as we have in FindTargetAndInterceptCourse() for missiles
//      so, we should use current barrel plane instead of object plane in order to calculate angles
//      in this case, we could stay with [-1, 1] for art sine, as we have in FindTargetAndInterceptCourse()
// NOTE NeedAngle should count on current 3d object rotation, since this is "additional" angle for barrel
bool GetTurretOnTargetOrientation(eObjectStatus ObjectStatus, // статус объекта, который целится
				  const sVECTOR3D &Location, // положение точки относительно которой будем наводить
				  const sVECTOR3D &CurrentObjectRotation, // текущие углы объекта
				  const float (&RotationMatrix)[9], // матрица вращения объекта
				  sVECTOR3D &NeedAngle, // нужные углы, чтобы получить нужное направление
				  int WeaponType) // номер оружия
{
	NeedAngle = CurrentObjectRotation;
	sVECTOR3D TargetLocation{Location};
	float TargetDist2{1000.0f * 1000.0f};
	bool TargetLocked{false};

	// horizontal plane (up/down)
	sVECTOR3D Orientation{0.0f, 0.0f, 1.0f};
	vw_Matrix33CalcPoint(Orientation, RotationMatrix);
	sVECTOR3D PointRight{1.0f, 0.0f, 0.0f};
	vw_Matrix33CalcPoint(PointRight, RotationMatrix);
	float A, B, C, D;
	vw_GetPlaneABCD(A, B, C, D, Location, Location + Orientation, Location + PointRight);

	ForEachSpaceShip([&] (const cSpaceShip &tmpShip) {
		if (NeedCheckCollision(tmpShip) &&
		    ObjectsStatusFoe(ObjectStatus, tmpShip.ObjectStatus)) {

			sVECTOR3D tmpLocation = tmpShip.GeometryCenter;
			vw_Matrix33CalcPoint(tmpLocation, tmpShip.CurrentRotationMat);
			sVECTOR3D RealLocation = tmpShip.Location + tmpLocation;

			// находим, за какое время снаряд долетит до объекта сейчас
			sVECTOR3D TTT = Location - RealLocation;
			float ProjectileSpeed = GetProjectileSpeed(WeaponType);
			if (ObjectStatus == eObjectStatus::Enemy)
				ProjectileSpeed = ProjectileSpeed / GameEnemyWeaponPenalty;
			float CurrentDist = TTT.Length();
			float ObjCurrentTime = CurrentDist / ProjectileSpeed;

			// находим где будет объект, когда пройдет это время (+ сразу половину считаем!)
			sVECTOR3D FutureLocation = tmpShip.Orientation ^ (tmpShip.Speed * ObjCurrentTime);
			// учитываем камеру...
			sVECTOR3D CamPosTTT{0.0f,0.0f,0.0f};
			if (tmpShip.ObjectStatus == eObjectStatus::Player)
				CamPosTTT = GameCameraMovement ^ (GameCameraGetSpeed() * ObjCurrentTime);

			// находи точку по середине... это нам и надо... туда целимся...
			sVECTOR3D PossibleRealLocation = RealLocation + FutureLocation + CamPosTTT;

			TTT = Location - PossibleRealLocation;
			float PossibleDist = TTT.Length();
			float PoprTime = PossibleDist/ProjectileSpeed;

			FutureLocation = tmpShip.Orientation ^ (tmpShip.Speed * PoprTime);
			// учитываем камеру...
			CamPosTTT = sVECTOR3D{0.0f, 0.0f, 0.0f};
			if (tmpShip.ObjectStatus == eObjectStatus::Player)
				CamPosTTT = GameCameraMovement ^ (GameCameraGetSpeed() * PoprTime);

			RealLocation = RealLocation + FutureLocation + CamPosTTT;

			// проверяем, если все точки выше
			float tmp1 = A * (RealLocation.x + tmpShip.OBB.Box[0].x) + B * (RealLocation.y + tmpShip.OBB.Box[0].y) + C * (RealLocation.z + tmpShip.OBB.Box[0].z) + D;
			float tmp2 = A * (RealLocation.x + tmpShip.OBB.Box[1].x) + B * (RealLocation.y + tmpShip.OBB.Box[1].y) + C * (RealLocation.z + tmpShip.OBB.Box[1].z) + D;
			float tmp3 = A * (RealLocation.x + tmpShip.OBB.Box[2].x) + B * (RealLocation.y + tmpShip.OBB.Box[2].y) + C * (RealLocation.z + tmpShip.OBB.Box[2].z) + D;
			float tmp4 = A * (RealLocation.x + tmpShip.OBB.Box[3].x) + B * (RealLocation.y + tmpShip.OBB.Box[3].y) + C * (RealLocation.z + tmpShip.OBB.Box[3].z) + D;
			float tmp5 = A * (RealLocation.x + tmpShip.OBB.Box[4].x) + B * (RealLocation.y + tmpShip.OBB.Box[4].y) + C * (RealLocation.z + tmpShip.OBB.Box[4].z) + D;
			float tmp6 = A * (RealLocation.x + tmpShip.OBB.Box[5].x) + B * (RealLocation.y + tmpShip.OBB.Box[5].y) + C * (RealLocation.z + tmpShip.OBB.Box[5].z) + D;
			float tmp7 = A * (RealLocation.x + tmpShip.OBB.Box[6].x) + B * (RealLocation.y + tmpShip.OBB.Box[6].y) + C * (RealLocation.z + tmpShip.OBB.Box[6].z) + D;
			float tmp8 = A * (RealLocation.x + tmpShip.OBB.Box[7].x) + B * (RealLocation.y + tmpShip.OBB.Box[7].y) + C * (RealLocation.z + tmpShip.OBB.Box[7].z) + D;

			if ((tmp1 > 0.0f) && (tmp2 > 0.0f) && (tmp3 > 0.0f) && (tmp4 > 0.0f) &&
			    (tmp5 > 0.0f) && (tmp6 > 0.0f) && (tmp7 > 0.0f) && (tmp8 > 0.0f)) {
				float TargetDist2TMP = (Location.x - RealLocation.x) * (Location.x - RealLocation.x) +
						       (Location.y - RealLocation.y) * (Location.y - RealLocation.y) +
						       (Location.z - RealLocation.z) * (Location.z - RealLocation.z);

				// проверяем, чтобы объект находился не ближе чем MinDistance
				if (TargetDist2 > TargetDist2TMP) {
					// запоминаем эти данные
					TargetLocation = RealLocation;
					TargetDist2 = TargetDist2TMP;
					TargetLocked = true;
				}
			}
		}
	});

	if (!TargetLocked)
		return false;

	// находим угол между плоскостью и прямой
	float m = TargetLocation.x - Location.x;
	float n = TargetLocation.y - Location.y;
	float p = TargetLocation.z - Location.z;

	// поправки к существующим углам поворота оружия
	float sss1 = vw_sqrtf(m * m + n * n + p * p);
	float sss2 = vw_sqrtf(A * A + B * B + C * C);
	if ((sss1 > 0.0f) && (sss2 > 0.0f)) {
		float ttt = (A * m + B * n + C * p) / (sss1 * sss2);
		vw_Clamp(ttt, -1.0f, 1.0f); // arc sine is computed in the interval [-1, +1]
		NeedAngle.x = asinf(ttt) * RadToDeg;
	}

	// find target location point projection onto horizontal plane
	if (sss2 > 0.0f) {
		float tmpDistanceToPlane = fabs(A * TargetLocation.x + B * TargetLocation.y + C * TargetLocation.z + D) / sss2;
		// reuse TargetLocation for point projection onto horizontal plane
		TargetLocation.x = TargetLocation.x - tmpDistanceToPlane * A;
		TargetLocation.y = TargetLocation.y - tmpDistanceToPlane * B;
		TargetLocation.z = TargetLocation.z - tmpDistanceToPlane * C;
		m = TargetLocation.x - Location.x;
		n = TargetLocation.y - Location.y;
		p = TargetLocation.z - Location.z;
		sss1 = vw_sqrtf(m * m + n * n + p * p);
	}
	if (sss1 == 0.0f)
		return true;

	// vertical plane (ahead/behind)
	sVECTOR3D PointUp{0.0f, 1.0f, 0.0f};
	vw_Matrix33CalcPoint(PointUp, RotationMatrix);
	float A2, B2, C2, D2;
	vw_GetPlaneABCD(A2, B2, C2, D2, Location, Location + PointRight, Location + PointUp);

	// vertical plane (left/right)
	float A3, B3, C3, D3;
	vw_GetPlaneABCD(A3, B3, C3, D3, Location, Location + Orientation, Location + PointUp);
	float sss3 = vw_sqrtf(A3 * A3 + B3 * B3 + C3 * C3);
	if (sss3 == 0.0f)
		return true;

	if ((A2 * TargetLocation.x +
	     B2 * TargetLocation.y +
	     C2 * TargetLocation.z + D2) <= 0.0f) {
		float ttt = (A3 * m + B3 * n + C3 * p) / (sss1 * sss3);
		vw_Clamp(ttt, -1.0f, 1.0f); // arc sine is computed in the interval [-1, +1]
		NeedAngle.y = 180.0f - asinf(ttt) * RadToDeg;
	} else {
		float ttt = (A3 * m + B3 * n + C3 * p) / (sss1 * sss3);
		vw_Clamp(ttt, -1.0f, 1.0f); // arc sine is computed in the interval [-1, +1]
		NeedAngle.y = asinf(ttt) * RadToDeg;
		if (NeedAngle.y < 0.0f)
			NeedAngle.y += 360.0f;
	}

	return true;
}

} // astromenace namespace
} // viewizard namespace