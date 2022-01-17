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
#define TARGET_FPS 30



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
Camera camera(glm::vec3(0.868368f, 3.01393f, 3.93507f), glm::vec3(-0.173054f, 0.805921f, -0.566166f),-107.4f,-36.3f);

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
static float roomPosition[3]={ -11.0f,-0.5f,6.0f };
float roomScale = 0.03;

//axes properties
glm::vec4 axisZColor(0.0f, 0.0f, 1.0f, 1.0f); //blue
glm::vec4 axisYColor(0.0f, 1.0f, 0.0f, 1.0f); //green
glm::vec4 axisXColor(1.0f, 0.0f, 0.0f, 1.0f); //red

//eye properties
struct eye_angle {
	float x;
	float y;
};
float eye_height = 1.659;

using namespace std;



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
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Simple Window");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("Load different input");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Eye direction line", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Eyeball", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::InputFloat3("Room Position", roomPosition);
		ImGui::InputFloat("Room Scale", &roomScale);

		ImGui::SliderFloat("eye height", &eye_height, 0.0f, 10.0f);
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

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
	glfwSetCursorPosCallback(window, mouse_callback);
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
	Shader person1Shader("anim_model_vs.glsl", "anim_model_fs.glsl");
	Shader person2Shader("anim_model_vs.glsl", "anim_model_fs.glsl");
	Shader person3Shader("anim_model_vs.glsl", "anim_model_fs.glsl");
	Shader normalShader("normal_vs.glsl", "normal_fs.glsl");
	Shader eyeShader("eye_vs.glsl", "normal_fs.glsl");
	Shader roomShader("model_loading.vs", "model_loading.fs");
	// load models
	// -----------

	//Model person1("../Resources/person1/person1.dae"); 
	//Animation person1_animation("../Resources/person1/person1.dae", &person1);
	//Animator person1_animator(&person1_animation);

	//Model person2("../Resources/person2/person2.dae");
	//Animation person2_animation("../Resources/person2/person2.dae", &person2);
	//Animator person2_animator(&person2_animation);
	Model person1("../Resources/male3/male3.dae"); 
	Animation person1_animation("../Resources/male3/male3.dae", &person1);
	Animator person1_animator(&person1_animation);

	Model person2("../Resources/male3/male3.dae");
	Animation person2_animation("../Resources/male3/male3.dae", &person2);
	Animator person2_animator(&person2_animation);
	Model person3("../Resources/male3/male3.dae");
	Animation person3_animation("../Resources/male3/male3.dae", &person3);
	Animator person3_animator(&person3_animation);

	Model room("../Resources/big-room/Room.obj");

	cout << "finish loading all object and animation" << endl;


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



	
	/*float aaa[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	glm::mat4 bbb;
	memcpy(glm::value_ptr(bbb), aaa, sizeof(aaa));
	cout << "bbb: " << glm::to_string(bbb) << endl;
	*/

	//auto head1 = (&person1_animation)->FindBone("mixamorig_Head");
	//auto head1 = (&person1_animation)->FindBone("Head");
	//auto head2 = (&person2_animation)->FindBone("Head");
	//auto head3 = (&person3_animation)->FindBone("Head");
	//for (int i = 0; i < (&person3_animation)->getBones().size(); i++)
	//	cout << (&person3_animation)->getBones()[i].GetBoneName() << endl;

	queue<eye_angle> eye1;
	queue<eye_angle> eye2;
	queue<eye_angle> eye3;
	std::ifstream file;
	std::string str;
	for (int i = 0; i < 3; i++)
	{
		

		//if(i==0)
		//	file = std::ifstream("Head_rotate_1.txt");
		//else if(i==1)
		//	file = std::ifstream("Head_rotate_2.txt");
		//else
		//	file = std::ifstream("Head_rotate_3.txt");

		//
		//
		//while (std::getline(file, str))
		//{
		//	float aaa[16]; int aindex = 0;
		//	size_t pos = 0; aaa[12] = 0, aaa[13] = 0, aaa[14] = 0, aaa[15] = 1;
		//	string token; string delimiter = ",";
		//	while (((pos = str.find(delimiter)) != std::string::npos) && aindex < 12) {
		//		token = str.substr(0, pos);
		//		//std::cout << token << std::endl;
		//		aaa[aindex] = stof(str); aindex++;
		//		str.erase(0, pos + delimiter.length());
		//	}
		//	glm::mat4 bbb;
		//	memcpy(glm::value_ptr(bbb), aaa, sizeof(aaa));
		//	//cout << "bbb: " << glm::to_string(bbb) << endl;
		//	if (i == 0)
		//		head1->setRotation(bbb);
		//	else if (i == 1)
		//	{
		//		head2->setRotation(bbb);
		//	}
		//	else
		//	{
		//		head3->setRotation(bbb);

		//	}
		//		
		//	
		//}

		if (i == 0)
			file = std::ifstream("head_eye_angle1.txt");
		else if (i == 1)
			file = std::ifstream("head_eye_angle2.txt");
		else
			file = std::ifstream("head_eye_angle3.txt");


		while (std::getline(file, str))
		{
			float x, y;
			size_t pos = 0;
			string token;
			string delimiter = ","; pos = str.find(delimiter); token = str.substr(0, pos); str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter); token = str.substr(0, pos); str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter); token = str.substr(0, pos); str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter); token = str.substr(0, pos); x = stof(str); str.erase(0, pos + delimiter.length());
			pos = str.find(delimiter); token = str.substr(0, pos); y = stof(str); str.erase(0, pos + delimiter.length());
			eye_angle a; a.x = x; a.y = y;
			if (i == 0)
				eye1.push(a);
			else if (i == 1)
				eye2.push(a);
			else
				eye3.push(a);
			

		}

	}

	//cout << "head2 stuff: " << glm::to_string(head2->get_current_rotation()) << endl;
	//cout << "head3: " << glm::to_string(head3->get_current_rotation()) << endl;



	vector<Bone*> myBone1; vector<Bone*> myBone2; vector<Bone*> myBone3;

	/*myBone.push_back((&person1_animation)->FindBone("mixamorig_Hips"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_Spine"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_Spine1"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_Spine2"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_Neck"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_Head"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_LeftShoulder"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_LeftArm"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_LeftForeArm"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_LeftHand"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_RightShoulder"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_RightArm"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_RightForeArm"));
	myBone.push_back((&person1_animation)->FindBone("mixamorig_RightHand"));*/
	myBone1.push_back((&person1_animation)->FindBone("Hips"));myBone1.push_back((&person1_animation)->FindBone("Spine"));
	myBone1.push_back((&person1_animation)->FindBone("Spine1"));myBone1.push_back((&person1_animation)->FindBone("Spine2"));
	myBone1.push_back((&person1_animation)->FindBone("Neck"));myBone1.push_back((&person1_animation)->FindBone("Head"));
	myBone1.push_back((&person1_animation)->FindBone("LeftShoulder"));myBone1.push_back((&person1_animation)->FindBone("LeftArm"));
	myBone1.push_back((&person1_animation)->FindBone("LeftForeArm"));myBone1.push_back((&person1_animation)->FindBone("LeftHand"));
	myBone1.push_back((&person1_animation)->FindBone("RightShoulder"));myBone1.push_back((&person1_animation)->FindBone("RightArm"));
	myBone1.push_back((&person1_animation)->FindBone("RightForeArm"));myBone1.push_back((&person1_animation)->FindBone("RightHand"));

	myBone2.push_back((&person2_animation)->FindBone("Hips")); myBone2.push_back((&person2_animation)->FindBone("Spine"));
	myBone2.push_back((&person2_animation)->FindBone("Spine1")); myBone2.push_back((&person2_animation)->FindBone("Spine2"));
	myBone2.push_back((&person2_animation)->FindBone("Neck")); myBone2.push_back((&person2_animation)->FindBone("Head"));
	myBone2.push_back((&person2_animation)->FindBone("LeftShoulder")); myBone2.push_back((&person2_animation)->FindBone("LeftArm"));
	myBone2.push_back((&person2_animation)->FindBone("LeftForeArm")); myBone2.push_back((&person2_animation)->FindBone("LeftHand"));
	myBone2.push_back((&person2_animation)->FindBone("RightShoulder")); myBone2.push_back((&person2_animation)->FindBone("RightArm"));
	myBone2.push_back((&person2_animation)->FindBone("RightForeArm")); myBone2.push_back((&person2_animation)->FindBone("RightHand"));

	myBone3.push_back((&person3_animation)->FindBone("Hips")); myBone3.push_back((&person3_animation)->FindBone("Spine"));
	myBone3.push_back((&person3_animation)->FindBone("Spine1")); myBone3.push_back((&person3_animation)->FindBone("Spine2"));
	myBone3.push_back((&person3_animation)->FindBone("Neck")); myBone3.push_back((&person3_animation)->FindBone("Head"));
	myBone3.push_back((&person3_animation)->FindBone("LeftShoulder")); myBone3.push_back((&person3_animation)->FindBone("LeftArm"));
	myBone3.push_back((&person3_animation)->FindBone("LeftForeArm")); myBone3.push_back((&person3_animation)->FindBone("LeftHand"));
	myBone3.push_back((&person3_animation)->FindBone("RightShoulder")); myBone3.push_back((&person3_animation)->FindBone("RightArm"));
	myBone3.push_back((&person3_animation)->FindBone("RightForeArm")); myBone3.push_back((&person3_animation)->FindBone("RightHand"));


	//for (int i = 0; i < myBone.size(); i++)
	//	cout << myBone[i]->GetBoneName() << endl;

	for (int i = 0; i < 3; i++) {

		if (i == 0)
			file = std::ifstream("Anson.txt");
		else if (i == 1)
			file = std::ifstream("Jonathan.txt");
		else
			file = std::ifstream("Stephan.txt");
		int k = 0;
		while (std::getline(file, str))
		{
			float x, y, z;
			size_t pos = 0;
			string token;
			string delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); str.erase(0, pos + delimiter.length());
			for (int j = 0; j < 14; j++)
			{


				delimiter = "  "; pos = str.find(delimiter); token = str.substr(0, pos); x = stof(str); str.erase(0, pos + delimiter.length());
				delimiter = "  "; pos = str.find(delimiter); token = str.substr(0, pos); y = stof(str); str.erase(0, pos + delimiter.length());
				delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); z = stof(str); str.erase(0, pos + delimiter.length());

				if (k % 4 == 0) {
					glm::mat4 bbb; bbb = glm::mat4(1.0f);


					if (j < 6) { //spine
						bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(1, 0, 0));
						bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(0, 1, 0));
						bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 0, 1));
					}
					else if (j < 10) { //left
						bbb = glm::rotate(bbb, -glm::radians(x), glm::vec3(0, 0, 1));
						bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
						bbb = glm::rotate(bbb, -glm::radians(z), glm::vec3(0, 1, 0));
					}
					else { //right
						bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
						bbb = glm::rotate(bbb, -glm::radians(y), glm::vec3(1, 0, 0));
						bbb = glm::rotate(bbb, -glm::radians(z), glm::vec3(0, 1, 0));
					}


					//cout << "bbb: " << glm::to_string(bbb) << endl;
					if (i == 0)
						myBone1[j]->setRotation(bbb);
					else if (i == 1)
						myBone2[j]->setRotation(bbb);
					else
						myBone3[j]->setRotation(bbb);
				}

				
			}

			k++;
		}
	}
	cout << "bone1s' size:" << myBone1[0]->get_all_rotation().size() << endl;
	cout << "bone2s' size:" << myBone2[0]->get_all_rotation().size() << endl;
	cout << "bone3s' size:" << myBone3[0]->get_all_rotation().size() << endl;

	//std::ifstream Anson("Anson.txt");
	//std::string str;
	////int showing = 0;
	//while (std::getline(Anson, str)) 
	//{
	//	float x, y, z;
	//	size_t pos = 0;
	//	string token;
	//	string delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); str.erase(0, pos + delimiter.length());
	//	for (int i = 0; i < 14; i++)
	//	{
	//		

	//		delimiter = "  "; pos = str.find(delimiter); token = str.substr(0, pos); x = stof(str); str.erase(0, pos + delimiter.length());
	//		delimiter = "  "; pos = str.find(delimiter); token = str.substr(0, pos); y = stof(str); str.erase(0, pos + delimiter.length());
	//		delimiter = " "; pos = str.find(delimiter); token = str.substr(0, pos); z = stof(str); str.erase(0, pos + delimiter.length());


	//		
	//		glm::mat4 bbb; bbb = glm::mat4(1.0f);


	//		if (i < 6) { //spine
	//			bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(1, 0, 0));
	//			bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(0, 1, 0));
	//			bbb = glm::rotate(bbb, glm::radians(z), glm::vec3(0, 0, 1));
	//		}
	//		else if (i < 10) { //left
	//			bbb = glm::rotate(bbb, -glm::radians(x), glm::vec3(0, 0, 1));
	//			bbb = glm::rotate(bbb, glm::radians(y), glm::vec3(1, 0, 0));
	//			bbb = glm::rotate(bbb, -glm::radians(z), glm::vec3(0, 1, 0));
	//		}
	//		else { //right
	//			bbb = glm::rotate(bbb, glm::radians(x), glm::vec3(0, 0, 1));
	//			bbb = glm::rotate(bbb, -glm::radians(y), glm::vec3(1, 0, 0));
	//			bbb = glm::rotate(bbb, -glm::radians(z), glm::vec3(0, 1, 0));
	//		}
	//		
	//		
	//		//cout << "bbb: " << glm::to_string(bbb) << endl;
	//		myBone[i]->setRotation(bbb);
	//	}
	//	

	//}

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

		person1_animator.UpdateAnimation(deltaTime);
		person2_animator.UpdateAnimation(deltaTime);
		person3_animator.UpdateAnimation(deltaTime);

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

		normalShader.setVec4("color", axisZColor); //red
		glBindVertexArray(lineZVAO);
		glDrawArrays(GL_LINES, 0, 2);
		normalShader.setVec4("color", axisXColor); //blue
		glBindVertexArray(lineXVAO);
		glDrawArrays(GL_LINES, 0, 2);
		normalShader.setVec4("color", axisYColor); //green
		glBindVertexArray(lineYVAO);
		glDrawArrays(GL_LINES, 0, 2);


		float character_scale = 0.5;
		person1Shader.use();

		// view/projection transformations
		person1Shader.setMat4("projection", projection);
		person1Shader.setMat4("view", view);
		auto transforms1 = person1_animator.GetPoseTransforms();
		for (int i = 0; i < transforms1.size(); ++i)
		{
			person1Shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms1[i]);

		}
		
		// render the loaded model
		glm::mat4 person1_model = glm::mat4(1.0f);
		person1_model = glm::translate(person1_model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		person1_model = glm::scale(person1_model, glm::vec3(character_scale*2));	// it's a bit too big for our scene, so scale it down
		person1Shader.setMat4("model", person1_model);
		person1.Draw(person1Shader);


		person2Shader.use();
		person2Shader.setMat4("projection", projection);
		person2Shader.setMat4("view", view);
		auto transforms2 = person2_animator.GetPoseTransforms();
		for (int i = 0; i < transforms2.size(); ++i)
			person2Shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms2[i]);


		// render the loaded model
		glm::mat4 person2_model = glm::mat4(1.0f);
		person2_model = glm::translate(person2_model, glm::vec3(1.0f, 0.0f, 1.732f)); // translate it down so it's at the center of the scene
		person2_model = glm::rotate(person2_model, glm::radians(-120.0f), glm::vec3(0.0, 1.0, 0.0));
		person2_model = glm::scale(person2_model, glm::vec3(character_scale*2));	// it's a bit too big for our scene, so scale it down
		person2Shader.setMat4("model", person2_model);
		person2.Draw(person2Shader);

		person3Shader.use();
		person3Shader.setMat4("projection", projection);
		person3Shader.setMat4("view", view);
		auto transforms3 = person3_animator.GetPoseTransforms();
		for (int i = 0; i < transforms3.size(); ++i)
			person3Shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms3[i]);


		// render the loaded model
		glm::mat4 person3_model = glm::mat4(1.0f);
		person3_model = glm::translate(person3_model, glm::vec3(-1.0f, 0.0f, 1.732f)); // translate it down so it's at the center of the scene
		person3_model = glm::rotate(person3_model, glm::radians(120.0f), glm::vec3(0.0, 1.0, 0.0));
		person3_model = glm::scale(person3_model, glm::vec3(character_scale*2));	// it's a bit too big for our scene, so scale it down
		person3Shader.setMat4("model", person3_model);
		person3.Draw(person3Shader);


		////draw eye line
		eyeShader.use();
		eyeShader.setMat4("projection", projection);
		eyeShader.setMat4("view", view);
		glLineWidth(5);

		glm::mat4 eyeAngle = glm::mat4(1.0f);
		eye_angle temp = eye1.front(); eye1.pop();
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 model_eye_angle = glm::mat4(1.0f);
		model_eye_angle = glm::translate(model_eye_angle, glm::vec3(0.0f, eye_height, 0.0f));
		model_eye_angle = glm::scale(model_eye_angle, glm::vec3(character_scale * 2));
		eyeShader.setMat4("eye_angle", eyeAngle);
		eyeShader.setMat4("headMatrix", myBone1[5]->get_current_rotation());
		eyeShader.setMat4("model", model_eye_angle);
		eyeShader.setVec4("color", axisZColor); //red
		glBindVertexArray(lineZVAO);
		glDrawArrays(GL_LINES, 0, 2);

		eyeAngle = glm::mat4(1.0f);
		temp = eye2.front(); eye2.pop();
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));
		model_eye_angle = glm::mat4(1.0f);
		model_eye_angle = glm::translate(model_eye_angle, glm::vec3(1.0f, eye_height, 1.732f));
		model_eye_angle = glm::rotate(model_eye_angle, glm::radians(-120.0f), glm::vec3(0.0, 1.0, 0.0));
		model_eye_angle = glm::scale(model_eye_angle, glm::vec3(character_scale * 2));
		eyeShader.setMat4("eye_angle", eyeAngle);
		eyeShader.setMat4("headMatrix", myBone2[5]->get_current_rotation());
		eyeShader.setMat4("model", model_eye_angle);
		eyeShader.setVec4("color", axisZColor); //red
		glBindVertexArray(lineZVAO);
		glDrawArrays(GL_LINES, 0, 2);

		eyeAngle = glm::mat4(1.0f);
		temp = eye3.front(); eye3.pop();
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));
		eyeAngle = glm::rotate(eyeAngle, -glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));

		model_eye_angle = glm::mat4(1.0f);
		model_eye_angle = glm::translate(model_eye_angle, glm::vec3(-1.0f, eye_height, 1.732f));
		model_eye_angle = glm::rotate(model_eye_angle, glm::radians(120.0f), glm::vec3(0.0, 1.0, 0.0));
		model_eye_angle = glm::scale(model_eye_angle, glm::vec3(character_scale * 2));
		eyeShader.setMat4("eye_angle", eyeAngle);
		eyeShader.setMat4("headMatrix", myBone3[5]->get_current_rotation());
		eyeShader.setMat4("model", model_eye_angle);
		eyeShader.setVec4("color", axisZColor); //red
		glBindVertexArray(lineZVAO);
		glDrawArrays(GL_LINES, 0, 2);

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

	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
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