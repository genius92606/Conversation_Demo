#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>


#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

int session;
int num_of_people = 3;
float sphere_scale = 0.4;

using namespace std;

//torso positions
vector<glm::vec3> init_hip_position;
std::vector<glm::vec3> allHipPositions[5];
float position_scale = 1000.0f;
vector<glm::vec3> init_spine_position;
std::vector<glm::vec3> allSpinePositions[5];
vector<glm::vec3> init_head_position;
std::vector<glm::vec3> allHeadPositions[5];

//torso rotations
std::vector<glm::mat4> allHipRotations[5];
std::vector<glm::mat4> allSpineRotations[5];
std::vector<glm::mat4> allHeadRotations[5];

struct eye_angle {
	float x;
	float y;
};
float eye_height = 1.667;
float eye_front = 0.198;
vector<vector<eye_angle> > eye_angles;
float head_front = 0.1;

//gaze behavior unit
glm::vec4 spherePositions[5];
glm::vec4 pupilPositions[5];
glm::vec4 gazeEnds[5];

//frame properties
int frames = 100000;



int main()
{
	string date;
	cout<< "Please enter date of capturing: ";
	cin >> date;
	cout << "Please enter session number: ";
	cin >> session;

	ofstream gazeBehavior("../3people/" + date + "/Session_" + to_string(session) + "_gazeBehavior.txt");

	// loading joints and eye-gaze angle
	// ---------------------------------------------------------------------------
	for (int i = 0; i < num_of_people; i++) {


		//loading joints (Mocap file)
		ifstream file = ifstream("../3people/" + date + "/Mocap/Separate/" + "Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_mocap_data.txt");
		string str;
		cout << "Loading pc " << i + 1 << endl;
		int k = 0;
		while (std::getline(file, str))
		{
			float x, y, z;
			size_t pos = 0;
			string token;
			string delimiter = " ";
			for (int j = 0; j < 22; j++)
			{


				delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); x = stof(str); str.erase(0, pos + delimiter.length());
				delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); y = stof(str); str.erase(0, pos + delimiter.length());
				delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); z = stof(str); str.erase(0, pos + delimiter.length());

				//Setup rotation for each joint based on different orientation

				//if (k % 2 == 0) { //cause orignial file is 60hz, I reduce it to 30hz
				glm::mat4 bbb; bbb = glm::mat4(1.0f);
				glm::mat4 ttt; ttt = glm::mat4(1.0f);
				if (j == 0) // hips rotations
				{

					bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
					bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
					bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 1, 0));
					allHipRotations[i].push_back(bbb);
				}
				else if (j == 9) //spine rotation
				{
					bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
					bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
					bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 1, 0));
					allSpineRotations[i].push_back(bbb);
				}
				else if (j == 10) //head rotation
				{
					bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
					bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
					bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 1, 0));
					allHeadRotations[i].push_back(bbb);
				}
				else if (j == 19) //hip position
				{
					if (k == 0)
						init_hip_position.push_back(glm::vec3(y, z, x));

					allHipPositions[i].push_back(glm::vec3(y, z - init_hip_position[i].y, x));

				}
				else if (j == 20) //spine1 position
				{
					if (k == 0)
						init_spine_position.push_back(glm::vec3(y, z, x));
					allSpinePositions[i].push_back(glm::vec3(y, z, x));

				}
				else if (j == 21) //head position
				{
					if (k == 0)
						init_head_position.push_back(glm::vec3(y, z, x));
					allHeadPositions[i].push_back(glm::vec3(y, z, x));

				}

			}
			k++;
		}

		if (allHipPositions[i].size() < frames)
			frames = allHipPositions[i].size();

		//cout << allHipPositions[i].size();

		//loading eye gaze
		file = ifstream("../Conversation/Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_EyeTracker_data.txt");
		//file = ifstream("../3people/" + date + "/Gaze/Convert/" + "Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_EyeTracker_data_gapfilled.txt");
		str.clear();
		string delimiter = " ";
		vector<eye_angle> angle_temps;
		while (std::getline(file, str))
		{
			eye_angle angle_temp;
			size_t pos = 0;
			string token;
			pos = str.find(delimiter); token = str.substr(0, pos); angle_temp.x = stof(str); str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter); token = str.substr(0, pos); angle_temp.y = stof(str); str.erase(0, pos + delimiter.length());

			angle_temps.push_back(angle_temp);
		}
		eye_angles.push_back(angle_temps);

	}

	cout << "Finsih loading all data and start calculating gaze behavior" << endl;
	//cout << "Frames: " << frames << endl;

	
	gazeBehavior << "P1 Gaze\tP2 Gaze\tP3 Gaze" << endl;

	for (int frame = 0;  frame < frames; frame++)
	{
		for (int i = 0; i < num_of_people; i++)
		{
			//global hips position
			glm::mat4 globalHips = glm::mat4(1.0f);
			globalHips = glm::translate(globalHips, allHipPositions[i][frame] / position_scale);
			float temp = init_hip_position[i].y / position_scale;
			globalHips = glm::translate(globalHips, glm::vec3(0.0f, temp, 0.0f));

			//spine rotation relate to hip
			glm::mat4 hipRotation = allHipRotations[i][frame];
			hipRotation = glm::translate(hipRotation, allSpinePositions[i][frame] / position_scale);

			//head rotation relate to spine
			glm::mat4 spineRotation = allSpineRotations[i][frame];
			spineRotation = glm::translate(spineRotation, allHeadPositions[i][frame] / position_scale);

			//raw eye gaze anagle
			glm::mat4 eyeAngle = glm::mat4(1.0f);
			//eyeAngle = glm::translate(eyeAngle, glm::vec3(0.0f, 0.0f, eye_front));
			eyeAngle = glm::rotate(eyeAngle, -glm::radians(eye_angles[i][frame].y), glm::vec3(1.0f, 0.0f, 0.0f));
			eyeAngle = glm::rotate(eyeAngle, -glm::radians(eye_angles[i][frame].x), glm::vec3(0.0f, 1.0f, 0.0f));
			//eye position relate to head center
			glm::mat4 eyeoffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, eye_front));

			pupilPositions[i] = globalHips * hipRotation * spineRotation *
				allHeadRotations[i][frame] * eyeoffset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			gazeEnds[i] = globalHips * hipRotation * spineRotation *
				allHeadRotations[i][frame] * eyeoffset * eyeAngle * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

			glm::mat4 sphere_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, head_front));
			sphere_model = glm::scale(sphere_model, glm::vec3(sphere_scale));
			sphere_model = globalHips * hipRotation * spineRotation* sphere_model;

			spherePositions[i] = sphere_model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			int r1 = (i + 1) % num_of_people;
			int r2 = (i + 2) % num_of_people;
			auto U = glm::normalize(glm::vec3(gazeEnds[i] - pupilPositions[i]));
			auto Q = glm::vec3(pupilPositions[i] - spherePositions[r1]);
			auto b = 2.0f * glm::dot(U, Q);
			auto c = glm::dot(Q, Q) - sphere_scale * sphere_scale;
			auto d1 = b * b - 4.0f * c;

			Q = glm::vec3(pupilPositions[i] - spherePositions[r2]);
			b = 2.0f * glm::dot(U, Q);
			c = glm::dot(Q, Q) - sphere_scale * sphere_scale;
			auto d2 = b * b - 4.0f * c;

			//cout << "p" << i << " gaze behavior: ";

		
			int look = 0;
			string gb = "nothing";
			if (d1 >= 0) {
				gb = "looking left";
				look = 1;
			}

			if (d2 >= 0) {
				gb = "looking right";
				look = 2;
			}
				

			//string final = "p" + to_string(i) + " gaze behavior: " + gb;
			//gazeBehavior << final <<", ";
			if (i < 2)
				gazeBehavior << look << "\t";
			else
				gazeBehavior << look;
		}
		gazeBehavior << endl;
	}
	gazeBehavior.close();
	cout << "Finsih calculating gaze behavior" << endl;
	system("pause");
	return 0;
}