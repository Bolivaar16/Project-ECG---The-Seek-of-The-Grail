#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include <Physics/BoundingBox.h>
#include <Player/Projectile.h>
#include <Logic/GameManager.h>
#include <Player/Enemy.h>
#include "Resources/Imgui/imgui.h"
#include "Resources/Imgui/imgui_impl_glfw.h"
#include "Resources/Imgui/imgui_impl_opengl3.h"
#include <Player/QuestItem.h>

void processKeyboardInput ();

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Window window("The Seek of the Grail", 800, 800);
Camera camera;

//Collision boxes
BoundingBox playerBox;
BoundingBox enemyBox;
GameManager gameManager;

// Projectiles container
std::vector<Projectile> fireballs;
// Enemies container
std::vector<Enemy> enemies;
//Quest Items
QuestItem staff(glm::vec3(10.0f, 0.0f, 100.0f), glm::vec3(2.0f, 2.0f, 2.0f));
QuestItem grail(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(1.0f, 1.0f, 1.0f));


glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

std::vector<glm::vec3> ruinsPositions = {
	glm::vec3(5.0f, 0.0f,  5.0f),
	glm::vec3(-5.0f, 0.0f,  5.0f),
	glm::vec3(-8.0f, 0.0f, -8.0f),
	glm::vec3(8.0f, 0.0f, -6.0f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(2.0f, 0.0f,  15.0f)
};

void resetGame() {
	std::cout << "Restarting Game..." << std::endl;
	
	gameManager = GameManager();

	camera = Camera();

	enemies.clear();
	for (int i = 0; i < 5; i++) {
		glm::vec3 pos = glm::vec3(-10.0f + (i * 4.0f), 0.0f, -15.0f);
		enemies.push_back(Enemy(pos, EnemyType::ZOMBIE));
	}
	enemies.push_back(Enemy(glm::vec3(0.0f, 0.0f, -30.0f), EnemyType::BOSS));
	staff = QuestItem(glm::vec3(10.0f, 0.0f, 100.0f), glm::vec3(2.0f, 2.0f, 2.0f));
	staff.isActive = true; 
	staff.isCollected = false;
	grail = QuestItem(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	grail.isActive = false;  
	grail.isCollected = false;
	fireballs.clear();
}

int main()
{
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

	//building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
	
	//Textures
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/rock.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");
	GLuint texZombie = loadBMP("Resources/Textures/zombi.bmp");

	glEnable(GL_DEPTH_TEST);

	// Initialize ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 400");

	
	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	std::vector<Texture> zombieTextures;
	zombieTextures.push_back(Texture());
	zombieTextures[0].id = texZombie;
	zombieTextures[0].type = "texture_diffuse";


	//Mesh mesh(vert, ind, textures3);

	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", textures2);
	Mesh zombieMesh = loader.loadObj("Resources/Models/zombi.obj", zombieTextures);

	// Initialization
	// Player
	playerBox = BoundingBox(camera.getCameraPosition(), glm::vec3(100.0f, 200.0f, 100.0f));

	//Grail
	grail.isActive = false;

	// Zombi
	/*
	for (int i = 0; i < 5; i++) {
		// Spawning them in a line for testing
		glm::vec3 pos = glm::vec3(-10.0f + (i * 4.0f), 0.0f, -15.0f);
		enemies.push_back(Enemy(pos, EnemyType::ZOMBIE));
	}*/
	
	enemies.push_back(Enemy(glm::vec3(0.0f, 0.0f, -30.0f), EnemyType::BOSS));

	
	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!gameManager.isGameOver && !gameManager.gameFinished) {
			processKeyboardInput();

			//test mouse input
			if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				std::cout << "Pressing mouse button" << std::endl;
			}

			//Phisics and logic update

			playerBox.update(camera.getCameraPosition() - glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f));

			for (auto& enemy : enemies) {
				enemy.update(deltaTime, camera.getCameraPosition());
			}

			for (auto& ball : fireballs) {
				ball.update(deltaTime);
			}

			gameManager.update(camera.getCameraPosition(), enemies);

			// --- QUEST LOGIC UPDATE ---

			// 1. Sync Grail visibility with GameManager
			if (gameManager.grailSpawned && !grail.isCollected) {
				grail.isActive = true;
			}

			// --- QUEST ITEM COLLISIONS ---

			// Staff Collision
			if (staff.isActive && playerBox.checkCollision(staff.box)) {
				std::cout << "Staff collected!" << std::endl;

				staff.isCollected = true;
				staff.isActive = false; // Hide it

				if (gameManager.currentTaskIndex == 0) {
					gameManager.tasks[0].isCompleted = true;
				}
			}

			// Grail Collision
			if (grail.isActive && playerBox.checkCollision(grail.box)) {
				std::cout << "Holy Grail collected!" << std::endl;

				grail.isCollected = true;
				grail.isActive = false; // Hide it (put in inventory)

				if (gameManager.currentTaskIndex == 3) {
					gameManager.tasks[3].isCompleted = true;

					gameManager.grailSpawned = false;
				}
			}
		}
		else if (gameManager.isGameOver || gameManager.gameFinished) { // Added gameFinished to allow restart on win too

			static bool enterPressedLastFrame = false;
			bool enterPressedNow = window.isPressed(GLFW_KEY_ENTER);

			if (enterPressedNow && !enterPressedLastFrame) {
				resetGame();
			}

			enterPressedLastFrame = enterPressedNow;
		}

		// Check collisions

		for (auto& enemy : enemies) {
			if (enemy.isDead) continue;

			if (playerBox.checkCollision(enemy.box)) {
				std::cout << "GAME OVER: Touched by enemy!" << std::endl;
				gameManager.isGameOver = true; // Trigger Defeat
			}
		}

		
		for (auto& ball : fireballs) {
			if (!ball.isActive && staff.isCollected) continue;

			for (auto& enemy : enemies) {
				if (enemy.isDead) continue;

				if (ball.box.checkCollision(enemy.box)) {
					std::cout << "Enemy Hit!" << std::endl;

					// Damage the enemy
					enemy.takeDamage(50.0f);

					// Destroy the fireball
					ball.isActive = false;
					break;
				}
			}
		}

		 //// Code for the light ////

		sunShader.use();

		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		GLuint ObjectColorID = glGetUniformLocation(sunShader.getId(), "objectColor");

		//Test for one Obj loading = light source

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//White moon
		glUniform3f(ObjectColorID, 1.0f, 1.0f, 1.0f);

		sun.draw(sunShader);

		// Draw fireballs
		for (auto& ball : fireballs)
		{
			if (ball.isActive&& staff.isCollected)
			{
				glm::mat4 ModelMatrix = glm::mat4(1.0);
				ModelMatrix = glm::translate(ModelMatrix, ball.position);
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.2f));

				glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

				glUniform3f(ObjectColorID, 1.0f, 0.5f, 0.0f);

				sun.draw(sunShader);
			}
		}


		// Draw Quest Items

		// Draw Staff
		if (staff.isActive) {
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, staff.position);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniform3f(ObjectColorID, 0.6f, 0.4f, 0.2f); // Brown
			sun.draw(sunShader);
		}

		// Draw Grail
		if (grail.isActive) {
			ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, grail.position);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniform3f(ObjectColorID, 1.0f, 0.84f, 0.0f);
			sun.draw(sunShader);
		}


		shader.use();

		///// Test Obj files for box ////

		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -2.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);


		///// Test plane Obj file //////

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(100.0f, 1.0f, 100.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex2);

		plane.draw(shader);

		// Draw enemies
		for (auto& enemy : enemies) {
			if (enemy.isDead) continue;

			ModelMatrix = glm::mat4(1.0);

			ModelMatrix = glm::translate(ModelMatrix, enemy.position);
			
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


			if (enemy.type == EnemyType::BOSS) {
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30.0f));
			}
			else {
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10.0f));
			}

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			zombieMesh.draw(shader);
		}

		// ImGui Rendering Logic
		// ---------------------------------------------------------

		// Start a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Define the HUD Window properties
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
		ImGui::SetNextWindowBgAlpha(0.35f);

		//Begin the ImGui Window
		if (ImGui::Begin("TaskOverlay", NULL, window_flags))
		{
			if (gameManager.isGameOver) {
				// DEFEAT SCREEN UI
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "YOU DIED");
				ImGui::Text("The darkness has consumed you.");
				ImGui::Separator();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press ENTER to restart the game");
			}
			else if (gameManager.gameFinished) {
				// VICTORY SCREEN UI
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "VICTORY!");
				ImGui::Text("Camelot is saved.");
			}
			else {
				// NORMAL HUD
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CURRENT OBJECTIVE:");
				std::string currentTask = gameManager.getCurrentTaskInfo();
				ImGui::Text("%s", currentTask.c_str());
			}
		}
		ImGui::End();

		//Render ImGui data onto the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.update();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void processKeyboardInput()
{
	float cameraSpeed = 30 * deltaTime;

	//translation
	if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
	if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
		camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	if (window.isPressed(GLFW_KEY_R))
		camera.keyboardMoveUp(cameraSpeed);
	// Shooting mechanic
	static float lastShotTime = 0.0f;
	float currentTime = glfwGetTime();
	if (window.isPressed(GLFW_KEY_F) && (currentTime - lastShotTime > 0.5f)) // 0.5s cooldown
	{
		// Spawn fireball at camera position, moving in camera direction
		fireballs.push_back(Projectile(camera.getCameraPosition() - glm::vec3(10.0f, 0.5f, 10.0f), camera.getCameraViewDirection()));
		lastShotTime = currentTime;
		std::cout << "Fireball cast!" << std::endl;
	}


	float rotationSpeed = 60 * deltaTime;

	//rotation
	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(rotationSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-rotationSpeed);
	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(rotationSpeed);
	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-rotationSpeed);
}
