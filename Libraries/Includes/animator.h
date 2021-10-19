#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <animation.h>
#include <bone.h>

class Animator
{
public:
	Animator(Animation* current)
	{
		m_CurrentAnimation = current;
		m_CurrentTime = 0.0;
		m_Transforms.reserve(100);
		for (int i = 0; i < 100; i++)
			m_Transforms.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{

				
			if (nodeName.compare("mixamorig_Head") == 0 || nodeName.compare("Head") == 0)
				//Bone->Update(m_CurrentTime);
				Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_Hips") == 0)
			//	Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_Spine") == 0)
			//	Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_Spine1") == 0)
			//	Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_Spine2") == 0)
			//	Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_Neck") == 0)
			//	Bone->Update_fromFile();
			//else if (nodeName.compare("mixamorig_LeftShoulder") == 0)
			//	Bone->Update_fromFile_left_arms();
			//else if (nodeName.compare("mixamorig_LeftArm") == 0)
			//{
			//	Bone->Update_fromFile_left_arms();
			//}								
			//else if (nodeName.compare("mixamorig_LeftForeArm") == 0)
			//{
			//	Bone->Update_fromFile_left_arms();
			//}
			//else if (nodeName.compare("mixamorig_LeftHand") == 0)
			//	Bone->Update_fromFile_left_arms();
			//else if (nodeName.compare("mixamorig_RightShoulder") == 0)
			//	Bone->Update_fromFile_right_arms();
			//else if (nodeName.compare("mixamorig_RightArm") == 0)
			//{
			//	glm::mat4 rotatee(1.0f);
			//	rotatee = glm::rotate(rotatee, glm::radians(30.0f), glm::vec3(0.0, 0.0, 1.0));
			//	//Bone->Update_fromFile_with_tune(rotatee);
			//	Bone->Update_fromFile_right_arms();
			//}
			//	
			//else if (nodeName.compare("mixamorig_RightForeArm") == 0)
			//	Bone->Update_fromFile_right_arms();
			//else if (nodeName.compare("mixamorig_RightHand") == 0)
			//	Bone->Update_fromFile_right_arms();
			else 
				Bone->Update_without_rotation(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
			
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_Transforms[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetPoseTransforms()
	{
		return m_Transforms;
	}

private:
	std::vector<glm::mat4> m_Transforms;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

};