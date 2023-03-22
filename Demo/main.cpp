#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//#include <filesystem.h>
#include <shader.h>
#include <camera.h>
//#include <model.h>
#include <animator.h>
#include <model_animation.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <sstream>
//#include <queue>
#define TARGET_FPS 60


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.868368f, 3.01393f, 3.93507f), glm::vec3(-0.173054f, 0.805921f, -0.566166f), -107.4f, -36.3f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Our state
bool show_demo_window = false;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//room properties
static float roomPosition[3] = { -7.0f,-0.06f,6.0f };
float roomScale = 0.02;

//axes properties
glm::vec4 axisZColor(0.0f, 0.0f, 1.0f, 1.0f); //blue
glm::vec4 axisYColor(0.0f, 1.0f, 0.0f, 1.0f); //green
glm::vec4 axisXColor(1.0f, 0.0f, 0.0f, 1.0f); //red

//eye properties
struct eye_angle {
	float x;
	float y;
};
float eye_height = 1.667;
float eye_front = 0.198;


vector<vector<eye_angle> > eye_angles;
bool show_eye = true;


//Interlocutor position properties
vector<glm::vec3> initPosition;
std::vector<glm::vec3> Sample_Positions[5];
float position_scale = 1000.0f;
vector<glm::vec3> init_spine_position;
std::vector<glm::vec3> allSpinePositions[5];
vector<glm::vec3> init_head_position;
std::vector<glm::vec3> allHeadPositions[5];

float head_front = 0.1;

vector<float> height_scale;

int session = 1;
int num_of_people = 3;
float character_scale = 1;

using namespace std;


//frame properties
int frame = 0;
int frames = 100000;

//sphere properties
bool show_sphere = true;
float sphere_scale = 0.4;


//gaze behavior unit
glm::vec4 spherePositions[5];
glm::vec4 pupilPositions[5];
glm::vec4 gazeEnds[5];
//std::vector<glm::vec4> spherePositions;
//std::vector<glm::vec4> pupilPositions;
//std::vector<glm::vec4> gazeEnds;
//std::vector<glm::vec4> eyeVectors;

void Gui() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{

		ImGui::Begin("Simple Window");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Load different input");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Other funcstions", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Eye direction line", &show_eye);      

		ImGui::Checkbox("Head sphere", &show_sphere);


		ImGui::InputFloat3("Room Position", roomPosition);
		ImGui::InputFloat("Room Scale", &roomScale);

		ImGui::SliderFloat("Sphere offset", &head_front, 0.0f, 2.0f);
		ImGui::SliderFloat("Eye offset", &eye_front, 0.0f, 3.5f);
		//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//	counter++;
		ImGui::SliderFloat("position scale", &position_scale, 100.0f, 1000.0f);
		ImGui::SliderInt("frame", &frame, 0, frames-1);

		ImGui::InputFloat("Sphere Scale", &sphere_scale);
		//ImGui::Text("counter = %d", frame);


		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	//// Rendering
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main()
{

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Three-party conversation", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// comment if you don't want the cursor movement
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	/// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);



	//tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
   //stbi_set_flip_vertically_on_load(true);

   // configure global opengl state
   // -----------------------------
	glEnable(GL_DEPTH_TEST);

	//// set up vertex data (and buffer(s)) and configure vertex attributes
 // // ------------------------------------------------------------------
	//float planeVertices[] = {
	//	// positions            // normals         // texcoords
	//	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	//	-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
	//	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

	//	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	//	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
	//	 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	//};
	//// plane VAO
	//unsigned int planeVAO, planeVBO;
	//glGenVertexArrays(1, &planeVAO);
	//glGenBuffers(1, &planeVBO);
	//glBindVertexArray(planeVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glBindVertexArray(0);

	////// load textures
 ////  // -------------
	//unsigned int floorTexture = loadTexture("../Resources/textures/wood.png");

	//Shader floorShader("model_loading.vs", "model_loading.fs");
	//floorShader.use();
	//floorShader.setInt("texture1", 0);

	//// build and compile shaders
	//// -------------------------
	Shader ourShader("model_loading.vs", "model_loading.fs");
	Shader skyboxShader("cubemaps.vs", "cubemaps.fs");
	Shader peopleShader("anim_model_vs.glsl", "anim_model_fs.glsl");
	Shader normalShader("normal_vs.glsl", "normal_fs.glsl");
	Shader eyeShader("eye_vs.glsl", "normal_fs.glsl");
	Shader roomShader("model_loading.vs", "model_loading.fs");


	Model room("../Resources/big-room/Room.obj");
	Model sphere("../Resources/sphere/sphere.obj");

	//cout << "finish loading all object and animation" << endl;


	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


		// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	float lineZ[] =
	{
		0.0f,  0.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	float lineX[] =
	{
		0.0f,  0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};
	float lineY[] =
	{
		0.0f,  0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	unsigned int lineZVAO, lineZVBO;
	glGenVertexArrays(1, &lineZVAO);
	glGenBuffers(1, &lineZVBO);
	glBindVertexArray(lineZVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineZVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineZ), &lineZ, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	unsigned int lineXVAO, lineXVBO;
	glGenVertexArrays(1, &lineXVAO);
	glGenBuffers(1, &lineXVBO);
	glBindVertexArray(lineXVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineXVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineX), &lineX, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	unsigned int lineYVAO, lineYVBO;
	glGenVertexArrays(1, &lineYVAO);
	glGenBuffers(1, &lineYVBO);
	glBindVertexArray(lineYVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineYVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineY), &lineY, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	vector<std::string> faces
	{
		"../Resources/skybox/right.jpg",
		"../Resources/skybox/left.jpg",
		"../Resources/skybox/top.jpg",
		"../Resources/skybox/bottom.jpg",
		"../Resources/skybox/front.jpg",
		"../Resources/skybox/back.jpg"

	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// shader configuration
	// --------------------

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	

	

	

	vector<Bone*>* myBone = new vector<Bone*>[num_of_people];

	// load models
	// -----------

	Model model("../Resources/male3/male3.dae");
	Animation* animations = new Animation[num_of_people];
	Animator* animators = new Animator[num_of_people];

	

	// loading joints and eye-gaze angle
	// ---------------------------------------------------------------------------
	for (int i = 0; i < num_of_people; i++) {

		animations[i] = Animation("../Resources/male3/male3.dae", &model);
		animators[i] = Animator(&animations[i]);

		myBone[i].push_back((&animations[i])->FindBone("Hips"));
		myBone[i].push_back((&animations[i])->FindBone("LeftUpLeg")); myBone[i].push_back((&animations[i])->FindBone("LeftLeg"));
		myBone[i].push_back((&animations[i])->FindBone("LeftFoot")); myBone[i].push_back((&animations[i])->FindBone("LeftToeBase"));
		myBone[i].push_back((&animations[i])->FindBone("RightUpLeg")); myBone[i].push_back((&animations[i])->FindBone("RightLeg"));
		myBone[i].push_back((&animations[i])->FindBone("RightFoot")); myBone[i].push_back((&animations[i])->FindBone("RightToeBase"));
		myBone[i].push_back((&animations[i])->FindBone("Spine1")); myBone[i].push_back((&animations[i])->FindBone("Head"));
		myBone[i].push_back((&animations[i])->FindBone("LeftShoulder")); myBone[i].push_back((&animations[i])->FindBone("LeftArm"));
		myBone[i].push_back((&animations[i])->FindBone("LeftForeArm")); myBone[i].push_back((&animations[i])->FindBone("LeftHand"));
		myBone[i].push_back((&animations[i])->FindBone("RightShoulder")); myBone[i].push_back((&animations[i])->FindBone("RightArm"));
		myBone[i].push_back((&animations[i])->FindBone("RightForeArm")); myBone[i].push_back((&animations[i])->FindBone("RightHand"));

		//loading joints
		//ifstream file = ifstream("../Conversation/4People/05-07-2022/Session_" + to_string(session) + "_PC_" + to_string(i+1) + "_mocap_data.txt");
		ifstream file = ifstream("../Conversation/Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_mocap_data.txt");

		string str;
		cout << "Loading pc " << i+1 << endl;
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
				glm::quat myQuat;
					//glm::mat4 bbb; bbb = glm::mat4(1.0f);
					if (j == 0)
					{

						/*bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
						bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
						bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 1, 0));*/
						
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x)));
					}
					else if (j == 12) //left arm
					{
						myQuat = glm::quat(glm::vec3(glm::radians(0.0), glm::radians(0.0), glm::radians(-90.0)));
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x))) *myQuat;
					}
						
					else if (j == 13) //left forearm
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x)));
					else if (j == 16) //right arm
					{
						myQuat = glm::quat(glm::vec3(glm::radians(0.0), glm::radians(0.0), glm::radians(90.0)));
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x))) * myQuat;
					}
					else if (j == 17) //right forearm
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x)));
					else if (j == 19) //hip position
					{
						if (k == 0)
							initPosition.push_back(glm::vec3(y, z, x));

						Sample_Positions[i].push_back(glm::vec3(y,z - initPosition[i].y,x));
	
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
					else
						myQuat = glm::quat(glm::vec3(glm::radians(y), glm::radians(z), glm::radians(x)));

					if (j < 19)
					{
						//myBone[i][j]->setRotation(bbb);
						myBone[i][j]->setRotation(glm::toMat4(myQuat));
						//newBone[j]->setPosition(glm::vec3(0, 0, 0));
						//cout << newBone[j]->GetBoneName() << ", x:" << x << ", y:" << y << ", z:" << z << endl;
					}


				//}
			}
			k++;
		}
		//determine frame
		if (myBone[i][0]->size() < frames)
			frames = myBone[i][0]->size();

		//Calculate the offset between real height and model height
		height_scale.push_back(eye_height / ((initPosition[i].y + init_spine_position[i].y + init_head_position[i].y) /
			position_scale));

		//loading eye gaze
		//file = ifstream("../Conversation/4People/05-07-2022/Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_EyeTracker_data.txt");
		file = ifstream("../Conversation/Session_" + to_string(session) + "_PC_" + to_string(i + 1) + "_EyeTracker_data.txt");

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
	
	//
	//auto yyy = (&animations[0])->GetBoneIDMap();
	//
	//cout << "head id: " << yyy["Head"].id << ", offset:" << glm::to_string(yyy["Head"].offset) << endl;
	//cout << "spine id:" << yyy["Spine1"].id << ", offset:" << glm::to_string(yyy["Spine1"].offset) << endl;
	//cout << "hips id:" << yyy["Hips"].id << ", offset:"<< glm::to_string(yyy["Hips"].offset) << endl;


	//for (int i = 0; i < (&animations[0])->getBones().size(); i++)
	//	cout << (&animations[0])->getBones()[i].GetBoneName() << endl;

	//auto dfs = (&animations[0])->getBones()[6].get_scales();
	//cout << "get scales: " << glm::to_string(dfs) << endl;

	//glm::mat4 temp = glm::mat4(1.0f);
	//temp = glm::scale(temp, glm::vec3(character_scale * 50));

	//cout << "scale: " << glm::to_string(temp) << endl;


	//auto see_mesh = model.meshes;
	//cout << "mesh size of model:" << see_mesh.size() << endl;
	//auto see_vertices = see_mesh[4].vertices;
	//cout << "vertices size for each mesh:" << see_vertices.size() << endl;
	//auto see_vertex = see_vertices[0];
	//cout << "Position: " << glm::to_string(see_vertex.Position) << endl;
	//cout << "bone ids: " << endl;
	//std::copy(std::begin(see_vertex.m_BoneIDs),
	//	std::end(see_vertex.m_BoneIDs),
	//	std::ostream_iterator<int>(std::cout, "\n"));
	//cout << "weights: "  << endl;
	//std::copy(std::begin(see_vertex.m_Weights),
	//	std::end(see_vertex.m_Weights),
	//	std::ostream_iterator<float>(std::cout, "\n"));

	////print out the initial head position of model (not related to our animation)
	//cout << "head position:" << glm::to_string(myBone[0][10]->getposition()[0].position) << endl;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{



		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//if (currentFrame > 20.0)
		//	break;
		// input
		// -----
		processInput(window);
		for (int i = 0; i < num_of_people; i++) {

			animators[i].UpdateAnimation(frame);
		}


		// render
		// ------
		glClearColor(0.32157f, 0.32157f, 0.32157f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//// draw objects
		//floorShader.use();
		//floorShader.setMat4("projection", projection);
		//floorShader.setMat4("view", view);
		//glm::mat4 model_floor = glm::mat4(1.0f);
		//model_floor = glm::translate(model_floor, glm::vec3(0.0f, 0.0f, 0.5f)); // translate it down so it's at the center of the scene
		//model_floor = glm::scale(model_floor, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		//floorShader.setMat4("model", model_floor);
		//// floor
		//glBindVertexArray(planeVAO);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, floorTexture);
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		//draw room
		roomShader.use();
		roomShader.setMat4("projection", projection);
		roomShader.setMat4("view", view);
		glm::mat4 model_room = glm::mat4(1.0f);
		model_room = glm::translate(model_room, glm::vec3(roomPosition[0], roomPosition[1], roomPosition[2])); // translate it down so it's at the center of the scene
		model_room = glm::scale(model_room, glm::vec3(roomScale, roomScale, roomScale));	// it's a bit too big for our scene, so scale it down
		roomShader.setMat4("model", model_room);
		room.Draw(roomShader);

		

		//draw axes
		normalShader.use();
		normalShader.setMat4("projection", projection);
		normalShader.setMat4("view", view);
		glLineWidth(4);

		glm::mat4 model_axis = glm::mat4(1.0f);


		model_axis = glm::scale(model_axis, glm::vec3(0.5f, 0.5f, 0.5f));
		normalShader.setMat4("model", model_axis);

		normalShader.setVec4("color", axisZColor); //blue
		glBindVertexArray(lineZVAO);
		glDrawArrays(GL_LINES, 0, 2);
		normalShader.setVec4("color", axisXColor); //red
		glBindVertexArray(lineXVAO);
		glDrawArrays(GL_LINES, 0, 2);
		normalShader.setVec4("color", axisYColor); //green
		glBindVertexArray(lineYVAO);
		glDrawArrays(GL_LINES, 0, 2);

		model_axis = glm::scale(model_axis, glm::vec3(1.0f));
		normalShader.setMat4("model", model_axis);

		

		//////Draw People//////
		
		for (int i = 0; i < num_of_people; i++) {

			peopleShader.use();

			// view/projection transformations
			peopleShader.setMat4("projection", projection);
			peopleShader.setMat4("view", view);

			auto transforms = animators[i].GetPoseTransforms();
			for (int k = 0; k < transforms.size(); ++k)
			{
				peopleShader.setMat4("finalBonesMatrices[" + std::to_string(k) + "]", transforms[k]);

			}
			// render the loaded model
			glm::mat4 person1_model = glm::mat4(1.0f);
			person1_model = glm::translate(person1_model, Sample_Positions[i][frame] / position_scale); // translate it down so it's at the center of the scene
			//person1_model = glm::translate(person1_model, glm::vec3(0.0, 0.0, 0.0));
			person1_model = glm::scale(person1_model, glm::vec3(character_scale/ height_scale[i]));	// it's a bit too big for our scene, so scale it down
			peopleShader.setMat4("model", person1_model);
			model.Draw(peopleShader);

			
			//global hips position
			glm::mat4 model_eye_angle = glm::mat4(1.0f);
			model_eye_angle = glm::translate(model_eye_angle, Sample_Positions[i][frame] / position_scale);
			model_eye_angle = glm::translate(model_eye_angle, glm::vec3(0.0f, initPosition[i].y / position_scale, 0.0f));
			
			//spine rotation relate to hip
			glm::mat4 hipRotation = myBone[i][0]->get_all_rotation()[frame];
			hipRotation = glm::translate(hipRotation, allSpinePositions[i][frame]  / position_scale);

			//head rotation relate to spine
			glm::mat4 spineRotation = myBone[i][9]->get_all_rotation()[frame];
			spineRotation = glm::translate(spineRotation, allHeadPositions[i][frame]  / position_scale);

			////draw eye line
			if (show_eye)
			{				
				eyeShader.use();
				eyeShader.setMat4("projection", projection);
				eyeShader.setMat4("view", view);

				//raw eye gaze anagle
				glm::mat4 eyeAngle = glm::mat4(1.0f);
				//eyeAngle = glm::translate(eyeAngle, glm::vec3(0.0f, 0.0f, eye_front));
				eyeAngle = glm::rotate(eyeAngle, -glm::radians(eye_angles[i][frame].y), glm::vec3(1.0f, 0.0f, 0.0f));
				eyeAngle = glm::rotate(eyeAngle, -glm::radians(eye_angles[i][frame].x), glm::vec3(0.0f, 1.0f, 0.0f));

				//scale of eye gaze line
				glm::mat4 scaleonly = glm::scale(glm::mat4(1.0f), glm::vec3(character_scale * 0.5));

				//eye position relate to head center
				glm::mat4 eyeoffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, eye_front));



				eyeShader.setMat4("eye_angle", eyeAngle);
				//eyeShader.setMat4("hipsMatrix", myBone[i][0]->get_all_rotation()[frame]); //hips rotaion
				//eyeShader.setMat4("spineMatrix", myBone[i][9]->get_all_rotation()[frame]);  //spine rotation
				eyeShader.setMat4("hipsMatrix", hipRotation); //hips rotaion
				eyeShader.setMat4("spineMatrix", spineRotation);  //spine rotation
				eyeShader.setMat4("headMatrix", myBone[i][10]->get_all_rotation()[frame]);  //head rotation
				eyeShader.setMat4("eyefront", eyeoffset);
				//eyeShader.setMat4("headtransform", transforms[2]); //head transform
				eyeShader.setMat4("model", model_eye_angle);
				eyeShader.setMat4("scaleonly", scaleonly);
				eyeShader.setVec4("color", axisZColor); //blue
				glLineWidth(5);
				glBindVertexArray(lineZVAO);
				glDrawArrays(GL_LINES, 0, 2);
				pupilPositions[i] = model_eye_angle * hipRotation * spineRotation * 
					myBone[i][10]->get_all_rotation()[frame] * eyeoffset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				gazeEnds[i] = model_eye_angle * hipRotation * spineRotation *
					myBone[i][10]->get_all_rotation()[frame] * eyeoffset * eyeAngle * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			}


			//draw sphere around head
			if (show_sphere)
			{
				glm::mat4 sphere_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, head_front));
				sphere_model = glm::scale(sphere_model, glm::vec3(sphere_scale));
				sphere_model = model_eye_angle * hipRotation * spineRotation* sphere_model;
				normalShader.use();
				normalShader.setMat4("projection", projection);
				normalShader.setMat4("view", view);
				normalShader.setMat4("model", sphere_model);
				normalShader.setVec4("color", glm::vec4(0.87f,0.67f,0.41f,0.5f)); //black
				sphere.Draw(normalShader);
				spherePositions[i] = sphere_model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			}
			

		}

		
		//cout << "p1 pupilposition: " << glm::to_string(pupilPositions[0]) << ", gazeEnds: " << glm::to_string(gazeEnds[0]) << endl;
		//cout << "p2 headposition: " << glm::to_string(spherePositions[1]) << endl;

		//p2 relate to p1
		auto U = glm::normalize(glm::vec3(gazeEnds[0] - pupilPositions[0]));

		auto Q = glm::vec3(pupilPositions[0] - spherePositions[1]);
		auto b = 2.0f * glm::dot(U,Q);
		auto c = glm::dot(Q, Q) - sphere_scale * sphere_scale;
		auto d = b * b - 4.0f * c;

		cout << d << ", ";
		if (d >= 0)
			cout << "Looking!!" << endl;
		else
			cout << "Not looking" << endl;
		

		//eyeAngle = glm::mat4(1.0f);
		////temp = eye2.front(); eye2.pop();
		//temp = eye2[frame];
		//eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));
		//eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));
		//model_eye_angle = glm::mat4(1.0f);
		//model_eye_angle = glm::translate(model_eye_angle, glm::vec3(1.0f, eye_height, 1.732f));
		//model_eye_angle = glm::rotate(model_eye_angle, glm::radians(-120.0f), glm::vec3(0.0, 1.0, 0.0));
		//model_eye_angle = glm::scale(model_eye_angle, glm::vec3(character_scale * 2));
		//eyeShader.setMat4("eye_angle", eyeAngle);
		//eyeShader.setMat4("headMatrix", myBone2[5]->get_current_rotation());
		//eyeShader.setMat4("model", model_eye_angle);
		//eyeShader.setVec4("color", axisZColor); //red
		//glBindVertexArray(lineZVAO);
		//glDrawArrays(GL_LINES, 0, 2);

		//eyeAngle = glm::mat4(1.0f);
		////temp = eye3.front(); eye3.pop();
		//temp = eye3[frame];
		//eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));
		//eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));

		//model_eye_angle = glm::mat4(1.0f);
		//model_eye_angle = glm::translate(model_eye_angle, glm::vec3(-1.0f, eye_height, 1.732f));
		//model_eye_angle = glm::rotate(model_eye_angle, glm::radians(120.0f), glm::vec3(0.0, 1.0, 0.0));
		//model_eye_angle = glm::scale(model_eye_angle, glm::vec3(character_scale * 2));
		//eyeShader.setMat4("eye_angle", eyeAngle);
		//eyeShader.setMat4("headMatrix", myBone3[5]->get_current_rotation());
		//eyeShader.setMat4("model", model_eye_angle);
		//eyeShader.setVec4("color", axisZColor); //red
		//glBindVertexArray(lineZVAO);
		//glDrawArrays(GL_LINES, 0, 2);

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		//cout <<"position"<< camera.Position[0] <<","<< camera.Position[1] << "," << camera.Position[2]<< endl;
		//cout << "front" << camera.Front[0] << "," << camera.Front[1] << "," << camera.Front[2] << endl;
		//cout << "up" << camera.Up[0] << "," << camera.Up[1] << "," << camera.Up[2] << endl;
		//cout << "yaw" << camera.Yaw<<endl;
		//cout << "pitch" << camera.Pitch << endl;
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		Gui();
		glfwSwapBuffers(window);
		glfwPollEvents();
		while ((glfwGetTime() - currentFrame) < 1.0 / TARGET_FPS) {
			// TODO: Put the thread to sleep, yield, or simply do nothing
		}
		frame++;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	delete[] myBone, animators, animations;

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.DirectlyUpdateCamera(glm::vec3(0.868368f, 3.01393f, 3.93507f), glm::vec3(-0.173054f, 0.805921f, -0.566166f), -107.4f, -36.3f);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}