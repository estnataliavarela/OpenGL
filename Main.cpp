#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

// Código fuente del vertex shader
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

// Código fuente del fragment shader
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"
"}\n\0";

int main()
{
	// Inicializamos GLFW
	glfwInit();

	// Configuramos la versión de OpenGL que vamos a utilizar
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Coordenadas de los tres vértices del triángulo
	GLfloat vertices[] =
	{
		-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,
		0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,
		0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f,
	};

	// Creamos la ventana
	GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);

	// Comprobamos que la ventana se haya creado correctamente
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Establecemos la ventana como contexto actual
	glfwMakeContextCurrent(window);

	// Cargamos las funciones de OpenGL
	gladLoadGL();

	// Definimos el área de visualización
	glViewport(0, 0, 800, 800);

	// Creamos y compilamos el vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Creamos y compilamos el fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Creamos el programa y unimos los dos shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Los shaders individuales ya no son necesarios
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Creamos el VAO y el VBO para almacenar los datos de los vértices
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Indicamos cómo están organizados los datos de cada vértice
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Establecemos el color de fondo
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	// Bucle principal de la aplicación
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Activamos el programa y dibujamos el triángulo
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);

		// Procesamos los eventos de la ventana
		glfwPollEvents();
	}

	// Liberamos los recursos utilizados
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	// Cerramos la ventana y finalizamos GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}