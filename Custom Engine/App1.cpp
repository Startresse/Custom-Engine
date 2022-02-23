#include "App1.h"

int App1::init()
{
    Shader shader("shaders/vertexApp1.glsl", "shaders/fragmentApp1.glsl");
    shaderProgram = shader.ID;

    // Set up vertex data (and buffer(s)) and attribute pointers

//#define TRIANGLE
#ifdef TRIANGLE

    GLfloat vertices[] =
    {
        // positions            // colors
        -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,         // Left
         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,         // Right
         0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,         // Top
    };

#else

    GLfloat vertices[] =
    {
        // positions            // colors
         0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f,         // Top Right
         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,         // Bottomt Right
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,         // Bottomt Left
        -0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 1.0f,         // Top Left
    };
    uint indices[] =
    {
        0, 1, 3,    // First triangle
        1, 2, 3,    // Second triangle
    };


    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#endif

    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray( VAO );
    
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
    
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 ); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
    
    glBindVertexArray( 0 ); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
    
	return 0;
}

int App1::render()
{
	// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
	glfwWaitEvents( );
	
	// Render
	// Clear the colorbuffer
	glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

    glm::mat4 trans = glm::mat4(1.0);
    //trans = glm::translate(trans, glm::vec3(0.5, 0.5, 0.0));

    if (key_state(GLFW_KEY_W))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glUseProgram( shaderProgram );

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray( VAO );
#ifdef TRIANGLE
	glDrawArrays( GL_TRIANGLES, 0, 3 );
#else
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
	glBindVertexArray( 0 );
	
	// Display calculated framebuffer (back to front) and prepares displayed framebuffer to be drawn (front to back)
	glfwSwapBuffers(window);

	return 0;
}

int App1::quit()
{
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );

	return 0;
}
