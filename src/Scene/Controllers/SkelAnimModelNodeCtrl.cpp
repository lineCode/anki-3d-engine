#include "SkelAnimModelNodeCtrl.h"
#include "SkelAnim.h"
#include "Skeleton.h"
#include "App.h"
#include "ModelNode.h"
#include "Model.h"
#include "MainRenderer.h"


//======================================================================================================================
// SkelAnimModelNodeCtrl                                                                                               =
//======================================================================================================================
SkelAnimModelNodeCtrl::SkelAnimModelNodeCtrl(ModelNode& modelNode_):
	Controller(CT_SKEL_ANIM),
	modelNode(modelNode_),
	frame(0.0)
{}


//======================================================================================================================
// interpolate                                                                                                         =
//======================================================================================================================
void SkelAnimModelNodeCtrl::interpolate(const SkelAnim& animation, float frame,
                                        Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	RASSERT_THROW_EXCEPTION(frame >= animation.framesNum);

	// calculate the t (used in slerp and lerp) using the keyframs and the frame and
	// calc the lPose and rPose witch indicate the pose ids in witch the frame lies between
	const Vec<uint>& keyframes = animation.keyframes;
	float t = 0.0;
	uint lPose = 0, rPose = 0;
	for(uint j=0; j<keyframes.size(); j++)
	{
		if((float)keyframes[j] == frame)
		{
			lPose = rPose = j;
			t = 0.0;
			break;
		}
		else if((float)keyframes[j] > frame)
		{
			lPose = j-1;
			rPose = j;
			t = (frame - (float)keyframes[lPose]) / float(keyframes[rPose] - keyframes[lPose]);
			break;
		}
	}

	// now for all bones update bone's poses
	RASSERT_THROW_EXCEPTION(boneRotations.size() < 1);
	for(uint i=0; i<boneRotations.size(); i++)
	{
		const SkelAnim::BoneAnim& banim = animation.bones[i];

		Mat3& localRot = boneRotations[i];
		Vec3& localTransl = boneTranslations[i];

		// if the bone has animations then slerp and lerp to find the rotation and translation
		if(banim.keyframes.size() != 0)
		{
			const SkelAnim::BonePose& lBpose = banim.keyframes[lPose];
			const SkelAnim::BonePose& rBpose = banim.keyframes[rPose];

			// rotation
			const Quat& q0 = lBpose.rotation;
			const Quat& q1 = rBpose.rotation;
			localRot = Mat3(q0.slerp(q1, t));

			// translation
			const Vec3& v0 = lBpose.translation;
			const Vec3& v1 = rBpose.translation;
			localTransl = v0.lerp(v1, t);
		}
		// else put the idents
		else
		{
			localRot = Mat3::getIdentity();
			localTransl = Vec3(0.0, 0.0, 0.0);
		}
	}
}


//======================================================================================================================
// updateBoneTransforms                                                                                                =
//======================================================================================================================
void SkelAnimModelNodeCtrl::updateBoneTransforms(const Skeleton& skeleton,
                                                 Vec<Vec3>& boneTranslations, Vec<Mat3>& boneRotations)
{
	uint queue[128];
	uint head = 0, tail = 0;

	// put the roots
	for(uint i=0; i<skeleton.bones.size(); i++)
	{
		if(skeleton.bones[i].parent == NULL)
		{
			queue[tail++] = i; // queue push
		}
	}

	// loop
	while(head != tail) // while queue not empty
	{
		uint boneId = queue[head++]; // queue pop
		const Skeleton::Bone& boned = skeleton.bones[boneId];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated transformation.
		combineTransformations(boneTranslations[boneId], boneRotations[boneId],
		                       boned.tslSkelSpaceInv, boned.rotSkelSpaceInv,
		                       boneTranslations[boneId], boneRotations[boneId]);

		combineTransformations(boned.tslSkelSpace, boned.rotSkelSpace,
		                       boneTranslations[boneId], boneRotations[boneId],
		                       boneTranslations[boneId], boneRotations[boneId]);

		// and finaly add the parent's transform
		if(boned.parent)
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			combineTransformations(boneTranslations[boned.parent->getPos()], boneRotations[boned.parent->getPos()],
		                         boneTranslations[boneId], boneRotations[boneId],
		                         boneTranslations[boneId], boneRotations[boneId]);
		}

		// now add the bone's childes
		for(uint i=0; i<boned.childsNum; i++)
		{
			queue[tail++] = boned.childs[i]->getPos();
		}
	}
}


//======================================================================================================================
// deform                                                                                                              =
//======================================================================================================================
void SkelAnimModelNodeCtrl::deform(const Skeleton& skeleton, const Vec<Vec3>& boneTranslations,
                                   const Vec<Mat3>& boneRotations, Vec<Vec3>& heads, Vec<Vec3>& tails)
{
	for(uint i=0; i<skeleton.bones.size(); i++)
	{
		const Mat3& rot = boneRotations[i];
		const Vec3& transl = boneTranslations[i];

		heads[i] = skeleton.bones[i].getHead().getTransformed(transl, rot);
		tails[i] = skeleton.bones[i].getTail().getTransformed(transl, rot);
	}
}


//======================================================================================================================
// update                                                                                                              =
//======================================================================================================================
void SkelAnimModelNodeCtrl::update(float)
{
	frame += step;
	if(frame > skelAnim->framesNum) // if the crnt is finished then play the next or loop the crnt
	{
		frame = 0.0;
	}

	interpolate(*skelAnim, frame, modelNode.getBoneTranslations(), modelNode.getBoneRotations());
	updateBoneTransforms(modelNode.getModel().getSkeleton(), modelNode.getBoneTranslations(), modelNode.getBoneRotations());
	if(app->getMainRenderer().getDbg().isEnabled() && app->getMainRenderer().getDbg().isShowSkeletonsEnabled())
	{
		deform(modelNode.getModel().getSkeleton(), modelNode.getBoneTranslations(), modelNode.getBoneRotations(),
		       modelNode.getHeads(), modelNode.getTails());
	}
}