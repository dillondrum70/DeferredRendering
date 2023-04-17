#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

#include "Material.h"
#include "Texture.h"
#include "TextureManager.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"

#include "FramebufferObject.h"

void passLightInfo(Shader* shader, glm::mat4 view, glm::mat4 projection);
void passTextureInfo(Shader* shader);

void drawScene(Shader* shader, glm::mat4 view, glm::mat4 projection, 
	ew::Mesh& cubeMesh, ew::Mesh& sphereMesh, ew::Mesh& cylinderMesh, ew::Mesh& planeMesh);

void drawLights(Shader* shader, glm::mat4 view, glm::mat4 projection, 
	ew::Mesh& cubeMesh, ew::Mesh& sphereMesh, ew::Mesh& cylinderMesh, ew::Mesh& planeMesh);

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

int SHADOWMASK_WIDTH = 1024;
int SHADOWMASK_HEIGHT = 1024;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

Material defaultMat;

glm::vec3 bgColor = glm::vec3(0);

float lightScale = .5f;

const int MAX_POINT_LIGHTS = 250;
PointLight pointLights[MAX_POINT_LIGHTS];
int pointLightCount = 10;
float pointLightRadius = 5.f;
float pointLightHeight = 2.f;

const int MAX_DIRECTIONAL_LIGHTS = 8;
DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
int directionalLightCount = 0;
float directionalLightAngle = 180.f;	//Angle towards center, 0 is down, + is towards the center, - is away from the center

const int MAX_SPOTLIGHTS = 8;
SpotLight spotlights[MAX_SPOTLIGHTS];
int spotlightCount = 0;
float spotlightRadius = 5.f;
float spotlightHeight = 5.f;
float spotlightAngle = 0.f;	//Angle towards center, 0 is down, + is towards the center, - is away from the center

float constantAttenuation = 1.f;
float linearAttenuation = .35f;
float quadraticAttenuation = .44f;

bool manuallyMoveLights = false;	//If true, allows you to move point lights manually

bool phong = true;

bool wireFrame = false;

bool gBufferQuadsEnabled = true;
bool deferredRenderingEnabled = true;

bool debugQuadEnabled = false;

const std::string ASSET_PATH = "./Textures/";
const std::string TEX_FILENAME_DIAMOND_PLATE = "DiamondPlate006C_4K_Color.jpg";
const std::string NORM_FILENAME_DIAMOND_PLATE = "DiamondPlate006C_4K_NormalGL.jpg";
const std::string SPECULAR_FILENAME_DIAMOND_PLATE = "DiamondPlate006C_4K_Specular.jpg";
const std::string TEX_FILENAME_PAVING_STONES = "PavingStones130_4K_Color.jpg";
const std::string NORM_FILENAME_PAVING_STONES = "PavingStones130_4K_NormalGL.jpg";

int currentTextureIndex = 0;

glm::vec3 brightColor = glm::vec3(1, 1, 1);
float brightnessThreshold = .95f;

TextureManager texManager;

ew::Transform cubeTransform;
ew::Transform sphereTransform;
ew::Transform planeTransform;
ew::Transform cylinderTransform;
ew::Transform lightTransform;
ew::Transform quadTransform;
ew::Transform debugQuadTransform;
ew::Transform positionQuadTransform;
ew::Transform normalQuadTransform;
ew::Transform albedoQuadTransform;
ew::Transform specularQuadTransform;

glm::vec3 shadowFrustumOrigin = glm::vec3(0, 0, 0);
glm::vec3 shadowFrustumExtents = glm::vec3(10, 10, 10);
float shadowDeathNearPlane = .001f;

float minBias = .005f;
float maxBias = .015f;

bool enablePCF = true;
bool enableSecondDepth = false;
int pcfSamples = 1;

glm::vec4 borderCol = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	Shader depthShader("shaders/depth.vert", "shaders/depth.frag");
	Shader depthToColorShader("shaders/blit.vert", "shaders/depthToColor.frag");

	Shader gBufferShader("shaders/defaultLit.vert", "shaders/gbuffer.frag");
	Shader albedoSpecShader("shaders/blit.vert", "shaders/albedoSpecBlit.frag");

	Shader deferredLitShader("shaders/blit.vert", "shaders/deferredLit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);
	ew::MeshData quadMeshData;
	ew::createQuad(2, 2, quadMeshData);
	ew::MeshData debugQuadMeshData;
	ew::createQuad(1, 1, debugQuadMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);
	ew::Mesh quadMesh(&quadMeshData);
	ew::Mesh debugQuadMesh(&debugQuadMeshData);

	ew::Mesh positionQuadMesh(&debugQuadMeshData);
	ew::Mesh normalQuadMesh(&debugQuadMeshData);
	ew::Mesh albedoQuadMesh(&debugQuadMeshData);
	ew::Mesh specularQuadMesh(&debugQuadMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Load in textures and add them to array
	
	texManager.AddTexture((ASSET_PATH + TEX_FILENAME_DIAMOND_PLATE).c_str());
	texManager.AddNormalMap((ASSET_PATH + NORM_FILENAME_DIAMOND_PLATE).c_str(), &texManager.textures[0]);
	texManager.AddSpecularMap((ASSET_PATH + SPECULAR_FILENAME_DIAMOND_PLATE).c_str(), &texManager.textures[0]);
	texManager.AddTexture((ASSET_PATH + TEX_FILENAME_PAVING_STONES).c_str());
	texManager.AddNormalMap((ASSET_PATH + NORM_FILENAME_PAVING_STONES).c_str(), &texManager.textures[1]);

	//Set texture samplers
	for (size_t i = 0; i < texManager.textureCount; i++)
	{
		//Set texture sampler to texture unit number
		litShader.setInt("_Textures[" + std::to_string(i) + "].texSampler", texManager.textures[i].texNumber);
		gBufferShader.setInt("_Textures[" + std::to_string(i) + "].texSampler", texManager.textures[i].texNumber);

		bool hasNormal = texManager.textures[i].GetNormalMap() != nullptr;
		litShader.setInt("_Textures[" + std::to_string(i) + "].hasNormal", hasNormal);
		gBufferShader.setInt("_Textures[" + std::to_string(i) + "].hasNormal", hasNormal);

		if (hasNormal)
		{
			litShader.setInt("_Textures[" + std::to_string(i) + "].normSampler", texManager.textures[i].GetNormalMap()->texNumber);
			gBufferShader.setInt("_Textures[" + std::to_string(i) + "].normSampler", texManager.textures[i].GetNormalMap()->texNumber);
		}

		bool hasSpecular = texManager.textures[i].GetSpecularMap() != nullptr;
		litShader.setInt("_Textures[" + std::to_string(i) + "].hasSpecular", hasSpecular);
		gBufferShader.setInt("_Textures[" + std::to_string(i) + "].hasSpecular", hasSpecular);

		if (hasSpecular)
		{
			litShader.setInt("_Textures[" + std::to_string(i) + "].specSampler", texManager.textures[i].GetSpecularMap()->texNumber);
			gBufferShader.setInt("_Textures[" + std::to_string(i) + "].specSampler", texManager.textures[i].GetSpecularMap()->texNumber);
		}
	}

	//Initialize shape transforms
	cubeTransform.position = glm::vec3(-2.0f, -.5f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	debugQuadTransform.position = glm::vec3(.5f, 0.5f, 0.0f);
	debugQuadTransform.scale = glm::vec3(.3f, .3f * (SCREEN_WIDTH / (float)SCREEN_HEIGHT), 1);

	positionQuadTransform.position = glm::vec3(.6f, 0.6f, 0.0f);
	positionQuadTransform.scale = glm::vec3(.2f, .2f, 1);
	normalQuadTransform.position = glm::vec3(.4f, 0.4f, 0.0f);
	normalQuadTransform.scale = glm::vec3(.2f, .2f, 1);
	albedoQuadTransform.position = glm::vec3(.4f, 0.6f, 0.0f);
	albedoQuadTransform.scale = glm::vec3(.2f, .2f, 1);
	specularQuadTransform.position = glm::vec3(.6f, 0.4f, 0.0f);
	specularQuadTransform.scale = glm::vec3(.2f, .2f, 1);

	pointLights[0].color = glm::vec3(1, 1, 1);
	pointLights[1].color = glm::vec3(0, 1, 1);
	pointLights[2].color = glm::vec3(0, 0, 1);
	pointLights[3].color = glm::vec3(1, 0, 1);
	pointLights[4].color = glm::vec3(1, 0, 0);
	pointLights[5].color = glm::vec3(1, .5, 0);
	pointLights[6].color = glm::vec3(1, 1, 0);
	pointLights[7].color = glm::vec3(0, 1, 0);

	spotlights[0].color = glm::vec3(1, 1, 1);
	spotlights[1].color = glm::vec3(0, 1, 1);
	spotlights[2].color = glm::vec3(0, 0, 1);
	spotlights[3].color = glm::vec3(1, 0, 1);
	spotlights[4].color = glm::vec3(1, 0, 0);
	spotlights[5].color = glm::vec3(1, .5, 0);
	spotlights[6].color = glm::vec3(1, 1, 0);
	spotlights[7].color = glm::vec3(0, 1, 0);

	directionalLights[0].color = glm::vec3(1, 1, 1);
	directionalLights[1].color = glm::vec3(0, 1, 1);
	directionalLights[2].color = glm::vec3(0, 0, 1);
	directionalLights[3].color = glm::vec3(1, 0, 1);
	directionalLights[4].color = glm::vec3(1, 0, 0);
	directionalLights[5].color = glm::vec3(1, .5, 0);
	directionalLights[6].color = glm::vec3(1, 1, 0);
	directionalLights[7].color = glm::vec3(0, 1, 0);

	Texture shadowDepthBuffer;
	shadowDepthBuffer.CreateTexture(GL_DEPTH_COMPONENT32F, SHADOWMASK_WIDTH, SHADOWMASK_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT);


	FramebufferObject shadowFbo;
	shadowFbo.Create();
	shadowFbo.SetDimensions(glm::vec2(SHADOWMASK_WIDTH, SHADOWMASK_HEIGHT));
	shadowFbo.AddDepthAttachment(shadowDepthBuffer);

	//If this is created after all texutres are loaded (and all the textures haven't been used) then it shouldn't overwrite or be overwritten by any textures
	Texture colorBuffer;
	colorBuffer.CreateTexture(GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT);

	Texture secondColorBuffer;
	secondColorBuffer.CreateTexture(GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT);
	
	RenderBuffer depthBuffer;
	depthBuffer.Create(SCREEN_WIDTH, SCREEN_HEIGHT);
	/*RenderBuffer depthBuffer;
	depthBuffer.Create(SCREEN_WIDTH, SCREEN_HEIGHT);*/

	FramebufferObject fbo;
	fbo.Create();
	fbo.AddColorAttachment(colorBuffer, GL_COLOR_ATTACHMENT0);
	fbo.AddColorAttachment(secondColorBuffer, GL_COLOR_ATTACHMENT1);
	fbo.AddDepthAttachment(depthBuffer);

	Shader* blitShader = new Shader("shaders/blit.vert", "shaders/blit.frag");
	PostprocessEffect noEffect = PostprocessEffect(blitShader, "None");
	fbo.AddEffect(&noEffect);

	Shader* grayscaleShader = new Shader("shaders/blit.vert", "shaders/grayscale.frag");
	GrayscaleEffect grayscaleEffect = GrayscaleEffect(grayscaleShader, "Grayscale");
	fbo.AddEffect(&grayscaleEffect);

	Shader* invertShader = new Shader("shaders/blit.vert", "shaders/invert.frag");
	InvertEffect invertEffect = InvertEffect(invertShader, "Invert");
	fbo.AddEffect(&invertEffect);

	Shader* edgeDetectShader = new Shader("shaders/blit.vert", "shaders/edgeDetect.frag");
	EdgeDetectEffect edgeDetectEffect = EdgeDetectEffect(edgeDetectShader, "Edge Detection");
	fbo.AddEffect(&edgeDetectEffect);

	Shader* chromaticShader = new Shader("shaders/blit.vert", "shaders/chromatic.frag");
	ChromaticEffect chromaticEffect = ChromaticEffect(chromaticShader, "Chromatic Aberration");
	fbo.AddEffect(&chromaticEffect);

	Shader* blurShader = new Shader("shaders/blit.vert", "shaders/blur.frag");
	BlurEffect blurEffect = BlurEffect(blurShader, "Gaussian Blur", &quadMesh);
	fbo.AddEffect(&blurEffect);

	//Blur shader and effect specific to the bloom effect
	Shader* blurBloomShader = new Shader("shaders/blit.vert", "shaders/blur.frag");
	BlurEffect blurBloomEffect = BlurEffect(blurBloomShader, "Gaussian Blur", &quadMesh);

	Shader* bloomShader = new Shader("shaders/blit.vert", "shaders/bloom.frag");
	BloomEffect bloomEffect = BloomEffect(bloomShader, "Bloom", &blurBloomEffect);
	fbo.AddEffect(&bloomEffect);

	Texture positionBuffer;
	positionBuffer.CreateTexture(GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT);
	Texture normalBuffer;
	normalBuffer.CreateTexture(GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_FLOAT);
	Texture albedoSpecularBuffer;
	albedoSpecularBuffer.CreateTexture(GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE);
	RenderBuffer gDepthBuffer;
	gDepthBuffer.Create(SCREEN_WIDTH, SCREEN_HEIGHT);

	FramebufferObject gBuffer;
	gBuffer.Create();
	gBuffer.AddColorAttachment(positionBuffer, GL_COLOR_ATTACHMENT0);
	gBuffer.AddColorAttachment(normalBuffer, GL_COLOR_ATTACHMENT1);
	gBuffer.AddColorAttachment(albedoSpecularBuffer, GL_COLOR_ATTACHMENT2);
	gBuffer.AddDepthAttachment(gDepthBuffer);
	
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		//Check framebuffer object is complete
		if (!gBuffer.IsComplete())
		{
			printf("GBuffer Framebuffer Object is incomplete\n\0");
		}

		//Check framebuffer object is complete
		if (!shadowFbo.IsComplete())
		{
			printf("Shadow Framebuffer Object is incomplete\n\0");
		}

		//Check framebuffer object is complete
		if (!fbo.IsComplete())
		{
			printf("Postprocessing Framebuffer Object is incomplete\n\0");
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		glm::mat4 lightView = glm::lookAt(shadowFrustumOrigin - (glm::normalize(directionalLights[0].dir) * shadowFrustumExtents.z), shadowFrustumOrigin, glm::vec3(0, 1, 0));
		glm::mat4 lightProjection = glm::ortho(-shadowFrustumExtents.x, shadowFrustumExtents.x, -shadowFrustumExtents.y, shadowFrustumExtents.y, shadowDeathNearPlane, shadowFrustumExtents.z * 2.f);

		if (!deferredRenderingEnabled)
		{
			////////////////////////// Shadows Disabled
			////Set up shadow mask framebuffer object
			//shadowFbo.Bind();
			//glClear(GL_DEPTH_BUFFER_BIT);
			//glDrawBuffer(GL_NONE);
			//glReadBuffer(GL_NONE);

			//if (enableSecondDepth)
			//{
			//	glCullFace(GL_FRONT);
			//}

			////Depth-only pass for shadow mask
			//drawScene(&depthShader, lightView, lightProjection, cubeMesh, sphereMesh, cylinderMesh, planeMesh);
		}
		

		

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();

		if (deferredRenderingEnabled)
		{
			//Bind the G Buffer
			gBuffer.Bind();
			gBuffer.Clear(bgColor, 0.0f);
			GLenum gBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, gBuffers);

			gBufferShader.use();

			passTextureInfo(&gBufferShader);

			drawScene(&gBufferShader, view, projection, cubeMesh, sphereMesh, cylinderMesh, planeMesh);

			fbo.Bind();
			fbo.Clear(bgColor);
			GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(2, buffers);

			deferredLitShader.use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, positionBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			deferredLitShader.setInt("_GBuffer.position", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			deferredLitShader.setInt("_GBuffer.normal", 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, albedoSpecularBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			deferredLitShader.setInt("_GBuffer.albedoSpecular", 2);

			passLightInfo(&deferredLitShader, view, projection);

			deferredLitShader.setMat4("_Model", ew::translate(quadTransform.position) * ew::scale(quadTransform.scale));
			quadMesh.draw();


			//unlitShader.use();

			//Draw light as a small sphere using unlit shader, ironically.
			//drawLights(&unlitShader, view, projection, cubeMesh, sphereMesh, cylinderMesh, planeMesh);
		}
		else
		{
			//Shadows + Forward Rendering
			//Bind and clear main frame buffer
			fbo.Bind();
			fbo.Clear(bgColor);
			GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(2, buffers);

			if(enableSecondDepth)
			{
				glCullFace(GL_BACK);
			}

			//Setup shadows in lit
			litShader.use();

			//texManager.BindTextures();	//This way outside of this step, all the texture slots can be used

			passTextureInfo(&litShader);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, shadowDepthBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			//Set border color to white to not change color
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderCol.x);
			litShader.setInt("_ShadowMap", 3);
			litShader.setMat4("_LightViewProj", lightProjection * lightView);
			drawScene(&litShader, view, projection, cubeMesh, sphereMesh, cylinderMesh, planeMesh);

			//Draw wireframe cube of shadow frustum
			
			unlitShader.use();

			//Draw light as a small sphere using unlit shader, ironically.
			drawLights(&unlitShader, view, projection, cubeMesh, sphereMesh, cylinderMesh, planeMesh);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);

			glm::mat4 frustPos = glm::translate(glm::mat4(1), shadowFrustumOrigin);
			glm::mat4 frustRot = glm::toMat4(glm::quatLookAt(glm::normalize(directionalLights[0].dir), glm::vec3(0, 1, 0)));
			glm::mat4 frustScale = glm::scale(glm::mat4(1), glm::vec3(2 * shadowFrustumExtents.x, 2 * shadowFrustumExtents.y, 2 * shadowFrustumExtents.z));
			

			unlitShader.setMat4("_Model", frustPos * frustRot * frustScale);
			unlitShader.setMat4("_View", view);
			unlitShader.setMat4("_Projection", projection);
			unlitShader.setVec3("_Color", glm::vec3(1, 1, 1));
			cubeMesh.draw();
		}

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);


		//Post Processing
		//After drawing, bind to default framebuffer, clear it, and draw the fullscreen quad with the shader
		fbo.Unbind(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT));
		glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Draw fullscreen quads with the current shader selected in fbo
		blitShader->setMat4("_Model", ew::translate(quadTransform.position) * ew::scale(quadTransform.scale));
		fbo.SetupShader();
		quadMesh.draw();


		//Debug Quads
		//Draw debug quad with shadow mask depth buffer
		if (debugQuadEnabled)
		{
			glActiveTexture(GL_TEXTURE0 + shadowDepthBuffer.GetTexture());
			glBindTexture(GL_TEXTURE_2D, shadowDepthBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

			depthToColorShader.use();
			depthToColorShader.setInt("_ColorTex", shadowDepthBuffer.GetTexture());
			//depthToColorShader.setVec2("_Offset", glm::vec2(debugQuadTransform.position.x, debugQuadTransform.position.y));
			//depthToColorShader.setFloat("_Near", shadowDeathNearPlane);
			//depthToColorShader.setFloat("_Far", 2 * shadowFrustumExtents.z);
			/*depthToColorShader.setFloat("_Width", debugQuadTransform.scale.x * SCREEN_WIDTH);
			depthToColorShader.setFloat("_Height", debugQuadTransform.scale.y * SCREEN_HEIGHT);*/
			depthToColorShader.setMat4("_Model", ew::translate(debugQuadTransform.position) * ew::scale(debugQuadTransform.scale));
			debugQuadMesh.draw();
		}

		//Draw debug quad with shadow mask depth buffer
		if (gBufferQuadsEnabled && deferredRenderingEnabled)
		{
			//Position
			glActiveTexture(GL_TEXTURE0 + positionBuffer.GetTexture());
			glBindTexture(GL_TEXTURE_2D, positionBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

			blitShader->use();
			blitShader->setInt("_ColorTex", positionBuffer.GetTexture());
			blitShader->setMat4("_Model", ew::translate(positionQuadTransform.position) * ew::scale(positionQuadTransform.scale));

			positionQuadMesh.draw();

			//Normal
			glActiveTexture(GL_TEXTURE0 + normalBuffer.GetTexture());
			glBindTexture(GL_TEXTURE_2D, normalBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

			blitShader->use();
			blitShader->setInt("_ColorTex", normalBuffer.GetTexture());
			blitShader->setMat4("_Model", ew::translate(normalQuadTransform.position) * ew::scale(normalQuadTransform.scale));

			normalQuadMesh.draw();
			
			//Albedo
			glActiveTexture(GL_TEXTURE0 + albedoSpecularBuffer.GetTexture());
			glBindTexture(GL_TEXTURE_2D, albedoSpecularBuffer.GetTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

			albedoSpecShader.use();
			albedoSpecShader.setVec2("_Offset", glm::vec2(0, 0));
			albedoSpecShader.setInt("_ColorTex", albedoSpecularBuffer.GetTexture());
			albedoSpecShader.setInt("_Albedo", true);
			albedoSpecShader.setMat4("_Model", ew::translate(albedoQuadTransform.position) * ew::scale(albedoQuadTransform.scale));

			albedoQuadMesh.draw();
			
			//Specular
			albedoSpecShader.setInt("_Albedo", false);
			albedoSpecShader.setMat4("_Model", ew::translate(specularQuadTransform.position) * ew::scale(specularQuadTransform.scale));

			specularQuadMesh.draw();
		}
		
		//Material
		defaultMat.ExposeImGui();
		
		//General Settings
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Settings");

		ImGui::Checkbox("Phong Lighting", &phong);
		ImGui::Checkbox("Manually Move Lights", &manuallyMoveLights);
		ImGui::Text("Opens option under settings\nin different types of lights\nto change the individual\nposition and/or direction of\nthe lights");

		ImGui::Text("GL Falloff Attenuation");
		ImGui::SliderFloat("Linear", &linearAttenuation, .0014f, 1.f);
		ImGui::SliderFloat("Quadratic", &quadraticAttenuation, .000007f, 2.0f);

		ImGui::Spacing();

		ImGui::Checkbox("Enable GBuffer Quads", &gBufferQuadsEnabled);

		ImGui::Spacing();

		ImGui::Checkbox("Enable Deferred Rendering", &deferredRenderingEnabled);

		ImGui::End();

		//Post Processing
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Post Processing");

		ImGui::SliderFloat3("Bloom Bright Color", &brightColor.x, 0, 1);
		ImGui::SliderFloat("Brightness Threshold", &brightnessThreshold, 0.0f, 1.0f);

		fbo.ExposeImGui();

		ImGui::End();

		//Shadow Mapping
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Shadow Mapping");

		ImGui::Checkbox("Enable Debug Quad", &debugQuadEnabled);

		ImGui::SliderFloat3("Shadow Frustum Origin", &shadowFrustumOrigin.x, -10, 10);
		ImGui::SliderFloat3("Shadow Frustum Extents", &shadowFrustumExtents.x, .01f, 20);

		ImGui::SliderFloat("Min Bias", &minBias, 0.0f, .1f);
		ImGui::SliderFloat("Max Bias", &maxBias, 0.0f, .1f);

		ImGui::Checkbox("Enable PCF", &enablePCF);
		ImGui::SliderInt("Smooth Shadow Samples", &pcfSamples, 1, 10);
		ImGui::Spacing();
		ImGui::Text("Use front or back faces for shadow mapping?");
		ImGui::Checkbox("Enable Second Depth Mapping", &enableSecondDepth);

		ImGui::End();

		//Point Lights
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Point Lights");

		ImGui::SliderInt("Light Count", &pointLightCount, 0, MAX_POINT_LIGHTS);

		if (!manuallyMoveLights)
		{
			ImGui::SliderFloat("Light Array Radius", &pointLightRadius, 0.f, 100.f);
			ImGui::SliderFloat("Light Array Height", &pointLightHeight, -5.f, 30.f);
		}

		for (size_t i = 0; i < pointLightCount; i++)
		{
			ImGui::Text(("Point Light" + std::to_string(i)).c_str());

			ImGui::PushID(i);
			pointLights[i].ExposeImGui(manuallyMoveLights);
			ImGui::PopID();
		}

		ImGui::End();

		//Directional Light
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Directional Light");

		ImGui::SliderInt("Light Count", &directionalLightCount, 0, MAX_DIRECTIONAL_LIGHTS);

		if (!manuallyMoveLights)
		{
			ImGui::SliderFloat("Light Array Angle", &directionalLightAngle, 90.f, 270.f);
		}
		
		for (size_t i = 0; i < directionalLightCount; i++)
		{
			ImGui::Text(("Directional Light " + std::to_string(i)).c_str());

			ImGui::PushID(i);
			directionalLights[i].ExposeImGui(manuallyMoveLights);
			ImGui::PopID();
		}

		ImGui::End();

		//Spotlight
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Spotlight");

		ImGui::SliderInt("Light Count", &spotlightCount, 0, MAX_SPOTLIGHTS);

		if (!manuallyMoveLights)
		{
			ImGui::SliderFloat("Light Array Radius", &spotlightRadius, 0.f, 100.f);
			ImGui::SliderFloat("Light Array Height", &spotlightHeight, -5.f, 30.f);
			ImGui::SliderFloat("Light Array Angle", &spotlightAngle, -60.f, 60.f);
		}

		for (size_t i = 0; i < spotlightCount; i++)
		{
			ImGui::Text(("Spotlight " + std::to_string(i)).c_str());

			ImGui::PushID(i);
			spotlights[i].ExposeImGui(manuallyMoveLights);
			ImGui::PopID();
		}

		ImGui::End();

		//Texture
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);	//Size to fit content
		ImGui::Begin("Textures");

		ImGui::SliderInt("Current Texture Channel", &currentTextureIndex, 0, texManager.textureCount - 1);

		for (size_t i = 0; i < texManager.textureCount; i++)
		{
			ImGui::PushID(i);
			ImGui::Text(("Texture" + std::to_string(i)).c_str());
			texManager.textures[i].ExposeImGui();
			ImGui::PopID();
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	delete bloomShader;
	delete blurBloomShader;
	delete blurShader;
	delete edgeDetectShader;
	delete invertShader;
	delete grayscaleShader;
	delete blitShader;

	glfwTerminate();
	return 0;
}

void passLightInfo(Shader* shader, glm::mat4 view, glm::mat4 projection)
{
	shader->use();
	shader->setVec3("_BrightColor", brightColor);	//Used in bloom
	shader->setFloat("_BrightnessThreshold", brightnessThreshold);	//Used in bloom
	shader->setMat4("_Projection", projection);
	shader->setMat4("_View", view);
	shader->setFloat("_MinBias", minBias);
	shader->setFloat("_MaxBias", maxBias);
	shader->setInt("_EnablePCF", enablePCF);
	shader->setInt("_PCFSamples", pcfSamples);

	//Attenuation Uniforms
	shader->setFloat("_Attenuation.constant", constantAttenuation);
	shader->setFloat("_Attenuation.linear", linearAttenuation);
	shader->setFloat("_Attenuation.quadratic", quadraticAttenuation);

	//Material Uniforms
	shader->setVec3("_Mat.color", defaultMat.color);
	shader->setFloat("_Mat.ambientCoefficient", defaultMat.ambientK);
	shader->setFloat("_Mat.diffuseCoefficient", defaultMat.diffuseK);
	shader->setFloat("_Mat.specularCoefficient", defaultMat.specularK);
	shader->setFloat("_Mat.shininess", defaultMat.shininess);
	shader->setFloat("_Mat.normalIntensity", defaultMat.normalIntensity);

	shader->setInt("_Phong", phong);

	//Point Light Uniforms
	shader->setInt("_UsedPointLights", pointLightCount);

	for (int i = 0; i < pointLightCount; i++)
	{
		if (!manuallyMoveLights)
		{
			pointLights[i].pos.x = pointLightRadius * (cos(2 * glm::pi<float>() * (i / (float)pointLightCount)));
			pointLights[i].pos.y = pointLightHeight;
			pointLights[i].pos.z = pointLightRadius * (sin(2 * glm::pi<float>() * (i / (float)pointLightCount)));
		}

		shader->setVec3("_PointLight[" + std::to_string(i) + "].pos", pointLights[i].pos);
		shader->setVec3("_PointLight[" + std::to_string(i) + "].color", pointLights[i].color);
		shader->setFloat("_PointLight[" + std::to_string(i) + "].intensity", pointLights[i].intensity);
	}

	//Directional Light Uniforms
	shader->setInt("_UsedDirectionalLights", directionalLightCount);

	for (int i = 0; i < directionalLightCount; i++)
	{
		if (!manuallyMoveLights)
		{
			float angle = glm::sin(glm::radians(-directionalLightAngle));

			directionalLights[i].dir = -glm::vec3(
				//Defines the direction this axis is rotated towards			Defines what angle to rotate by
				(cos(2 * glm::pi<float>() * (i / (float)directionalLightCount))) * angle,
				1,
				(sin(2 * glm::pi<float>() * (i / (float)directionalLightCount))) * angle
			);
		}

		shader->setVec3("_DirectionalLight[" + std::to_string(i) + "].dir", directionalLights[i].dir);
		shader->setVec3("_DirectionalLight[" + std::to_string(i) + "].color", directionalLights[i].color);
		shader->setFloat("_DirectionalLight[" + std::to_string(i) + "].intensity", directionalLights[i].intensity);
	}

	//Spotlight Uniforms
	shader->setInt("_UsedSpotlights", spotlightCount);

	for (int i = 0; i < spotlightCount; i++)
	{
		if (!manuallyMoveLights)
		{
			spotlights[i].pos.x = spotlightRadius * (cos(2 * glm::pi<float>() * (i / (float)spotlightCount)));
			spotlights[i].pos.y = spotlightHeight;
			spotlights[i].pos.z = spotlightRadius * (sin(2 * glm::pi<float>() * (i / (float)spotlightCount)));

			spotlights[i].dir = glm::vec3(
				//Defines the direction this axis is rotated towards			Defines what angle to rotate by
				(cos(2 * glm::pi<float>() * (i / (float)spotlightCount))) * glm::sin(glm::radians(-spotlightAngle)),
				-1,
				(sin(2 * glm::pi<float>() * (i / (float)spotlightCount))) * glm::sin(glm::radians(-spotlightAngle))
			);
		}

		shader->setVec3("_Spotlight[" + std::to_string(i) + "].pos", spotlights[i].pos);
		shader->setVec3("_Spotlight[" + std::to_string(i) + "].dir", spotlights[i].dir);
		shader->setVec3("_Spotlight[" + std::to_string(i) + "].color", spotlights[i].color);
		shader->setFloat("_Spotlight[" + std::to_string(i) + "].intensity", spotlights[i].intensity);
		shader->setFloat("_Spotlight[" + std::to_string(i) + "].range", spotlights[i].range);
		shader->setFloat("_Spotlight[" + std::to_string(i) + "].minAngle", glm::cos(glm::radians(spotlights[i].innerAngle)));
		shader->setFloat("_Spotlight[" + std::to_string(i) + "].maxAngle", glm::cos(glm::radians(spotlights[i].outerAngle)));
		shader->setFloat("_Spotlight[" + std::to_string(i) + "].falloff", spotlights[i].angleFalloff);
	}
}

void passTextureInfo(Shader* shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texManager.textures[currentTextureIndex].GetTexture());
	shader->setInt("_CurrentTexture.texSampler", 0);

	if (texManager.textures[currentTextureIndex].GetNormalMap())
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texManager.textures[currentTextureIndex].GetNormalMap()->GetTexture());
		shader->setInt("_CurrentTexture.normSampler", 1);
		shader->setInt("_CurrentTexture.hasNormal", true);
	}
	else
	{
		shader->setInt("_CurrentTexture.hasNormal", false);
	}

	if (texManager.textures[currentTextureIndex].GetSpecularMap())
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texManager.textures[currentTextureIndex].GetSpecularMap()->GetTexture());
		shader->setInt("_CurrentTexture.specSampler", 2);
		shader->setInt("_CurrentTexture.hasSpecular", true);
	}
	else
	{
		shader->setInt("_CurrentTexture.hasSpecular", false);
	}
}

//Author: Dillon Drummond
void drawScene(Shader* shader, glm::mat4 view, glm::mat4 projection, ew::Mesh& cubeMesh, ew::Mesh& sphereMesh, ew::Mesh& cylinderMesh, ew::Mesh& planeMesh)
{
	passLightInfo(shader, view, projection);

	//Textures
	shader->setInt("_CurrentTexture", currentTextureIndex);

	texManager.textures[currentTextureIndex].offset += texManager.textures[currentTextureIndex].scrollSpeed * deltaTime;
	shader->setVec2("_CurrentTexture.scaleFactor", texManager.textures[currentTextureIndex].scaleFactor);
	shader->setVec2("_CurrentTexture.offset", texManager.textures[currentTextureIndex].offset);

	shader->setVec3("_CamPos", camera.getPosition());

	//Draw cube
	glm::mat4 cubeModel = cubeTransform.getModelMatrix();
	shader->setMat4("_Model", cubeModel);
	shader->setMat4("_NormalMatrix", glm::transpose(glm::inverse(cubeModel)));
	cubeMesh.draw();

	////Draw sphere
	glm::mat4 sphereModel = sphereTransform.getModelMatrix();
	shader->setMat4("_Model", sphereModel);
	shader->setMat4("_NormalMatrix", glm::transpose(glm::inverse(sphereModel)));
	sphereMesh.draw();

	//Draw cylinder
	glm::mat4 cylinderModel = cylinderTransform.getModelMatrix();
	shader->setMat4("_Model", cylinderModel);
	shader->setMat4("_NormalMatrix", glm::transpose(glm::inverse(cylinderModel)));
	cylinderMesh.draw();

	//Draw plane
	glm::mat4 planeModel = planeTransform.getModelMatrix();
	shader->setMat4("_Model", planeModel);
	shader->setMat4("_NormalMatrix", glm::transpose(glm::inverse(planeModel)));
	planeMesh.draw();
}

void drawLights(Shader* shader, glm::mat4 view, glm::mat4 projection,
	ew::Mesh& cubeMesh, ew::Mesh& sphereMesh, ew::Mesh& cylinderMesh, ew::Mesh& planeMesh)
{
	shader->use();
	shader->setVec3("_BrightColor", brightColor);	//Used in bloom
	shader->setFloat("_BrightnessThreshold", brightnessThreshold);	//Used in bloom
	shader->setMat4("_Projection", projection);
	shader->setMat4("_View", view);
	for (size_t i = 0; i < pointLightCount; i++)
	{
		shader->setMat4("_Model", glm::translate(glm::mat4(1), pointLights[i].pos) * glm::scale(glm::mat4(1), glm::vec3(lightScale)));
		shader->setVec3("_Color", pointLights[i].color);
		sphereMesh.draw();
	}

	for (size_t i = 0; i < spotlightCount; i++)
	{
		glm::mat4 rotation = ew::rotateX(-asin(spotlights[i].dir.z)) * ew::rotateY(acos(spotlights[i].dir.y)) * ew::rotateZ(-asin(spotlights[i].dir.x));

		shader->setMat4("_Model", glm::translate(glm::mat4(1), spotlights[i].pos) * rotation * glm::scale(glm::mat4(1), glm::vec3(lightScale)));
		shader->setVec3("_Color", spotlights[i].color);
		cylinderMesh.draw();
	}
}

//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
