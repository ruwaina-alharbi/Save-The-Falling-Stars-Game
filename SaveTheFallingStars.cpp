/*
PROJECT: Save the Falling Stars

// ------------------------------------------------------------------------------------------------------------------
   USER MANUAL:
// ------------------------------------------------------------------------------------------------------------------

1. Build the Project:
   - Open the project in Visual Studio.
   - Make sure the source file is inside Source Files.
   - Make sure OpenGL/GLUT is linked correctly.
   - Click Build Solution or press Ctrl + Shift + B.

2. Run the Project:
   - After the build succeeds, press Ctrl + F5 or click Run.
   - A window titled "SAVE THE FALLING STARS" will appear.

3. Use the Main Menu:
   - Click START to open the game screen.
   - Click INSTRUCTIONS to view the game instructions.
   - Click CREDITS to view project information.
   - Click EXIT to close the program.

4. Navigation:
   - From the Instructions or Credits screen, click anywhere to return to the main menu.
   - The mouse is used to move between screens.

5. Notes:
   - The program is organized into separate functions for each screen (menu, game, instructions, credits, win, and lose).
   - A common background function is used to reuse the same design across multiple screens.
*/

// ------------------------------------------------------------------------------------------------------------------
// LIBRARIES
// ------------------------------------------------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h> // OpenGL core library for graphics rendering
#include <GL/glut.h>   // GLUT library for window management and user input
#else
#include <GL/glew.h>
#include <GL/freeglut.h>    // FreeGLUT library (Windows alternative to GLUT)
#endif
#include <stdlib.h>  // Standard library for random number generation (rand function)
#include <iostream>  // Input/output stream for console messages
#include <math.h>    // Math library for trigonometric functions (cos, sin) and fabs
#include <stdio.h>   // Standard I/O library for file operations and printf
using namespace std; // This allows us to use standard library names without the std:: prefix

// ------------------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
// ------------------------------------------------------------------------------------------------------------------

// Texture IDs for win and lose screens (loaded from BMP files)
GLuint winTexture;   // Stores the texture ID for the win screen image
GLuint loseTexture;  // Stores the texture ID for the lose screen image

// File paths for texture images
char winImagePath[] = "win.bmp";   // Path to win screen image (800x600 BMP file)
char loseImagePath[] = "lose.bmp"; // Path to lose screen image (800x600 BMP file)

// Game state enumeration - controls which screen is currently displayed
enum GameState { MENU, GAME, INSTRUCTIONS, CREDITS, WIN, LOSE };
GameState currentScreen = MENU; // Start at the main menu
/* Reference: https://www.w3schools.com/cpp/cpp_enum.asp
              https://learnopengl.com/In-Practice/2D-Game/Setting-up */

              // Falling star position (starts above the screen and falls down)
float starX = 0.0f;  // Horizontal position of the star (-1.0 to 1.0)
float starY = 1.1f;  // Vertical position of the star (starts above screen)

// Bomb position (starts above the screen, falls down)
float bombX = 0.5f;  // Horizontal position of the bomb
float bombY = 1.1f;  // Vertical position of the bomb (starts above screen)

// Player basket position (moves left and right with arrow keys)
float basketX = 0.0f; // Horizontal position of the basket (0.0 = center)

// Game progress tracking variables
int score = 0;         // Number of stars caught by the player
bool gameOver = false; // Flag to stop the game when player loses


// ------------------------------------------------------------------------------------------------------------------
// TEXTURE LOADING 
// ------------------------------------------------------------------------------------------------------------------
// Loads a BMP image file and converts it into an OpenGL texture
// Parameters: filename (path to BMP file), width and height (dimensions in pixels)
GLuint LoadTexture(const char* filename, int width, int height)
{
    GLuint texture;
    unsigned char* data;
    FILE* file;

    // Step 1: Open the BMP file in binary read mode
    file = fopen(filename, "rb");

    // Check if the file was opened successfully
    if (file == NULL)
    {
        cout << "Unable to open the image file: " << filename << endl;
        cout << "Program will exit :(" << endl;
        exit(0);
        return 0;
    }

    // Step 2: Allocate memory to store the image pixel data
    // Each pixel has 3 bytes (Red, Green, Blue)
    data = (unsigned char*)malloc(width * height * 3);

    // Read the raw pixel data from the BMP file
    fread(data, width * height * 3, 1, file);

    // Close the file after reading
    fclose(file);

    // Step 3: Convert BGR color format to RGB
    // BMP files store colors as BGR, but OpenGL expects RGB
    for (int i = 0; i < width * height; ++i)
    {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];      // Save Blue value
        R = data[index + 2];  // Save Red value

        data[index] = R;      // Swap Red to first position
        data[index + 2] = B;  // Swap Blue to third position
    }

    // Step 4: Generate an OpenGL texture and bind it
    glGenTextures(1, &texture);            // Create a new texture ID
    glBindTexture(GL_TEXTURE_2D, texture); // Activate this texture for editing

    // Step 5: Set texture environment mode (how texture colors mix with geometry colors)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Step 6: Set texture filtering parameters (how texture looks when scaled)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Smooth when shrunk
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Smooth when enlarged

    // Step 7: Set texture wrapping parameters (what happens at edges)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat horizontally
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat vertically

    // Step 8: Upload the pixel data to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Step 9: Free the memory we allocated for pixel data
    free(data);

    // Check for any OpenGL errors during texture creation
    if (glGetError() != GL_NO_ERROR)
        printf("GLError in LoadTexture()\n");

    return texture; // Return the texture ID for later use
}

// ------------------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS (Geometric Drawing) 
// ------------------------------------------------------------------------------------------------------------------
// Draws a smooth filled circle using polar coordinates
// Parameters: cx, cy (center position), r (radius)
// Uses cos/sin to calculate points around the circle
void drawFilledCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // Center point of the circle

    // Generate 100 points around the circle for a smooth appearance
    for (int i = 0; i <= 100; i++)
    {
        float angle = 2.0f * 3.14159f * i / 100.0f; // Calculate angle in radians
        float x = cx + cos(angle) * r;  // Calculate X position using cosine
        float y = cy + sin(angle) * r;  // Calculate Y position using sine
        glVertex2f(x, y);
    }

    glEnd();
}

// ------------------------------------------------------------------------------------------------------------------
// Renders text on the screen using GLUT bitmap font
// Parameters: x, y (position), text (string to display)
// Font: GLUT_BITMAP_TIMES_ROMAN_24
// ------------------------------------------------------------------------------------------------------------------
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y); // Set starting position for text

    // Loop through each character and render it
    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
}
/* Reference: https://www.lighthouse3d.com/tutorials/glut-tutorial/bitmap-fonts/
              https://www.opengl.org/resources/libraries/glut/spec3/node76.html */

              // ------------------------------------------------------------------------------------------------------------------
              //Draws a semi-transparent black box with a white border
              // Used for: Instructions and Credits screens to make text readable
              // ------------------------------------------------------------------------------------------------------------------
void drawTransparentBox() {
    // Enable alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the semi-transparent black box background
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f); // Black with 60% opacity
    glBegin(GL_QUADS);
    glVertex2f(-0.70f, 0.40f);   // Top left
    glVertex2f(0.70f, 0.40f);    // Top right
    glVertex2f(0.70f, -0.60f);   // Bottom right
    glVertex2f(-0.70f, -0.60f);  // Bottom left
    glEnd();

    // Draw the semi-transparent white border
    glColor4f(1.0f, 1.0f, 1.0f, 0.4f); // White with 40% opacity
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.70f, 0.40f);
    glVertex2f(0.70f, 0.40f);
    glVertex2f(0.70f, -0.60f);
    glVertex2f(-0.70f, -0.60f);
    glEnd();

    glDisable(GL_BLEND); // Disable blending after drawing
}


// ------------------------------------------------------------------------------------------------------------------
// INDIVIDUAL OBJECT DRAWING   
// ------------------------------------------------------------------------------------------------------------------

// Purpose: Draws a glowing moon using 3 layered circles
// Parameters: x, y (position of moon center)
// Creates a soft glow effect with decreasing circle sizes
// ------------------------------------------------------------------------------------------------------------------
void drawMoon(float x, float y) {
    // Layer 1: Outer soft glow (largest circle, grey-blue)
    glColor3f(0.75f, 0.75f, 0.82f);
    drawFilledCircle(x, y, 0.18f);

    // Layer 2: Middle tone (medium circle, light grey)
    glColor3f(0.86f, 0.86f, 0.90f);
    drawFilledCircle(x, y, 0.14f);

    // Layer 3: Bright core (smallest circle, near white)
    glColor3f(0.96f, 0.96f, 0.98f);
    drawFilledCircle(x, y, 0.10f);
}


// ------------------------------------------------------------------------------------------------------------------
// Purpose: Draws a cloud shape using overlapping circles
// Parameters: x, y (position), s (scale/size multiplier)
// 5 circles arranged to create a cloud silhouette
// ------------------------------------------------------------------------------------------------------------------
void drawCloud(float x, float y, float s) {
    // Main cloud body (dark grey-blue color)
    glColor3f(0.58f, 0.55f, 0.65f);

    // Draw 5 overlapping circles to form the cloud shape
    // Position formula: x ± offset*s, y ± offset*s
    drawFilledCircle(x, y, 0.08f * s);                      // Center circle
    drawFilledCircle(x + 0.08f * s, y - 0.03f * s, 0.10f * s); // Right-down
    drawFilledCircle(x + 0.18f * s, y, 0.08f * s);          // Far right
    drawFilledCircle(x + 0.25f * s, y - 0.02f * s, 0.06f * s); // Edge right
    drawFilledCircle(x + 0.12f * s, y + 0.03f * s, 0.09f * s); // Top bump

    // Add lighter highlight on top (creates depth effect)
    glColor3f(0.66f, 0.63f, 0.72f);
    drawFilledCircle(x + 0.05f * s, y, 0.04f * s);
}


// ------------------------------------------------------------------------------------------------------------------
// Draws a small decorative star with 8 points
// Parameters: x, y (center position)
// Filled star with a brighter outline for emphasis
// ------------------------------------------------------------------------------------------------------------------
void drawStar(float x, float y)
{
    // Draw filled star body (warm light yellow)
    glColor3f(1.0f, 0.95f, 0.70f);
    glBegin(GL_POLYGON);
    glVertex2f(x, y + 0.030f);          // Top point
    glVertex2f(x + 0.010f, y + 0.010f); // Top-right inner
    glVertex2f(x + 0.030f, y);          // Right point
    glVertex2f(x + 0.010f, y - 0.010f); // Bottom-right inner
    glVertex2f(x, y - 0.030f);          // Bottom point
    glVertex2f(x - 0.010f, y - 0.010f); // Bottom-left inner
    glVertex2f(x - 0.030f, y);          // Left point
    glVertex2f(x - 0.010f, y + 0.010f); // Top-left inner
    glEnd();

    // Draw star outline (brighter white-yellow for emphasis)
    glColor3f(1.0f, 1.0f, 0.85f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y + 0.030f);
    glVertex2f(x + 0.010f, y + 0.010f);
    glVertex2f(x + 0.030f, y);
    glVertex2f(x + 0.010f, y - 0.010f);
    glVertex2f(x, y - 0.030f);
    glVertex2f(x - 0.010f, y - 0.010f);
    glVertex2f(x - 0.030f, y);
    glVertex2f(x - 0.010f, y + 0.010f);
    glEnd();

    glLineWidth(1.0f); // Reset line width to default
}


// ------------------------------------------------------------------------------------------------------------------
// Draws an animated falling star (game object) with pointed rays
// Parameters: x, y (current position)
// 5-pointed star using triangle fan with alternating long/short rays
// ------------------------------------------------------------------------------------------------------------------
void drawFallingStar(float x, float y) {
    glColor3f(1.0f, 0.90f, 0.0f); // Bright golden yellow

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center point

    // Generate 10 points (5 outer points + 5 inner points) for star shape
    for (int i = 0; i <= 10; i++) {
        // Calculate angle for this point (starts from top, rotates clockwise)
        float angle = i * 36.0f * 3.14159f / 180.0f - (3.14159f / 2.0f);

        // Alternate between long rays (outer) and short rays (inner)
        float r = (i % 2 == 0) ? 0.12f : 0.05f;

        // Calculate point position using polar coordinates
        glVertex2f(x + cos(angle) * r, y + sin(angle) * r);
    }

    glEnd();
}


// ------------------------------------------------------------------------------------------------------------------
// Draws a cartoon-style bomb with fuse and spark effect
// Parameters: x, y (position of bomb center)
// Black sphere with fuse line and spark at the end
// ------------------------------------------------------------------------------------------------------------------
void drawBomb(float x, float y) {
    // Main bomb body (dark sphere)
    glColor3f(0.1f, 0.1f, 0.1f);
    drawFilledCircle(x, y, 0.09f);

    // Light reflection spot (creates 3D effect)
    glColor3f(0.4f, 0.4f, 0.4f);
    drawFilledCircle(x - 0.03f, y + 0.03f, 0.02f);

    // Draw fuse line (curved upward)
    glColor3f(0.6f, 0.3f, 0.1f); // Brown color
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y + 0.08f);        // Start at top of bomb
    glVertex2f(x + 0.02f, y + 0.12f); // Bend right
    glVertex2f(x + 0.05f, y + 0.15f); // End point
    glEnd();

    // Draw fire/spark at end of fuse
    glColor3f(1.0f, 0.5f, 0.0f); // Orange flame
    drawFilledCircle(x + 0.05f, y + 0.15f, 0.015f);

    // Draw spark lines radiating from flame
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow sparks
    glBegin(GL_LINES);
    glVertex2f(x + 0.05f, y + 0.15f); glVertex2f(x + 0.07f, y + 0.18f); // Up-right
    glVertex2f(x + 0.05f, y + 0.15f); glVertex2f(x + 0.08f, y + 0.15f); // Right
    glVertex2f(x + 0.05f, y + 0.15f); glVertex2f(x + 0.07f, y + 0.12f); // Down-right
    glEnd();

    glLineWidth(1.0f); // Reset line width
}


// ------------------------------------------------------------------------------------------------------------------
// Draws the player's basket that catches falling stars
// Parameters: x, y (base position of basket)
// Trapezoid shape with top border, moves with basketX global variable
// Uses glPushMatrix/glPopMatrix to apply translation without affecting other objects
// ------------------------------------------------------------------------------------------------------------------
void drawBasket(float x, float y) {
    glPushMatrix(); // Save current transformation state
    glTranslatef(basketX, 0.0f, 0.0f); // Move basket left/right based on player input

    // Main basket body (trapezoid shape)
    glColor3f(0.5f, 0.3f, 0.1f); // Dark brown
    glBegin(GL_QUADS);
    glVertex2f(x - 0.25f, y);        // Top left
    glVertex2f(x + 0.25f, y);        // Top right
    glVertex2f(x + 0.18f, y - 0.2f); // Bottom right (narrower)
    glVertex2f(x - 0.18f, y - 0.2f); // Bottom left (narrower)
    glEnd();

    // Top border/rim of basket (slightly wider for 3D effect)
    glColor3f(0.7f, 0.4f, 0.2f); // Lighter brown
    glBegin(GL_QUADS);
    glVertex2f(x - 0.27f, y + 0.04f); // Extends beyond basket width
    glVertex2f(x + 0.27f, y + 0.04f);
    glVertex2f(x + 0.27f, y);
    glVertex2f(x - 0.27f, y);
    glEnd();

    glPopMatrix(); // Restore original transformation state
}


// ------------------------------------------------------------------------------------------------------------------
//  FULL SCREEN DRAWING FUNCTIONS 
// ------------------------------------------------------------------------------------------------------------------

// Draws the starry night sky background used by multiple screens
// Used by: Menu, Instructions, Credits screens
// Elements: Gradient sky, moon, clouds, decorative stars
// ------------------------------------------------------------------------------------------------------------------
void commonBackground() {
    // Draw gradient sky background (dark blue bottom to dark purple top)
    glBegin(GL_QUADS);
    glColor3f(0.01f, 0.01f, 0.06f);  // Bottom color (dark blue)
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glColor3f(0.12f, 0.08f, 0.25f);  // Top color (dark purple)
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Draw the moon at top center
    drawMoon(0.0f, 0.76f);

    // Draw clouds at various positions and sizes
    drawCloud(-0.90f, 0.72f, 1.05f);
    drawCloud(-0.55f, 0.62f, 0.90f);
    drawCloud(-0.20f, 0.68f, 0.75f);
    drawCloud(0.15f, 0.74f, 0.95f);
    drawCloud(0.55f, 0.66f, 1.00f);
    drawCloud(0.85f, 0.60f, 0.70f);
    drawCloud(0.05f, 0.50f, 0.55f);

    // Scatter decorative stars across the sky
    drawStar(-0.80f, 0.92f); drawStar(-0.20f, 0.88f); drawStar(0.60f, 0.90f);
    drawStar(-0.70f, 0.62f); drawStar(-0.95f, 0.70f); drawStar(0.40f, 0.65f);
    drawStar(-0.50f, 0.35f); drawStar(-0.15f, 0.20f); drawStar(0.10f, 0.40f);
    drawStar(0.60f, 0.45f);  drawStar(-0.60f, -0.10f); drawStar(0.20f, -0.05f);
    drawStar(0.70f, -0.20f); drawStar(-0.30f, -0.60f); drawStar(0.30f, -0.65f);
    drawStar(0.80f, -0.50f);
}


// ------------------------------------------------------------------------------------------------------------------
// Draws the main menu screen with clickable buttons
// Buttons: START, INSTRUCTIONS, CREDITS, EXIT
// Design: Gradient buttons with title text at top
// ------------------------------------------------------------------------------------------------------------------
void drawMainMenu() {
    commonBackground(); // Draw starry night background

    // Game title at top center
    drawText(-0.36, 0.49f, "SAVE THE FALLING STARS");

    // Button colors:
    // Top edge: Lighter purple (0.42, 0.38, 0.55)
    // Bottom edge: Darker purple (0.22, 0.18, 0.32)
    // Text: Light cream (1.0, 1.0, 0.8)

    // START Button (y: 0.10 to 0.30)
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.38f, 0.55f);
    glVertex2f(-0.30f, 0.30f); glVertex2f(0.30f, 0.30f);
    glColor3f(0.22f, 0.18f, 0.32f);
    glVertex2f(0.30f, 0.10f); glVertex2f(-0.30f, 0.10f);
    glEnd();
    glColor3f(1.0f, 1.0f, 0.8f);
    drawText(-0.10f, 0.18f, "START");

    // INSTRUCTIONS Button (y: -0.18 to 0.02)
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.38f, 0.55f);
    glVertex2f(-0.30f, 0.02f); glVertex2f(0.30f, 0.02f);
    glColor3f(0.22f, 0.18f, 0.32f);
    glVertex2f(0.30f, -0.18f); glVertex2f(-0.30f, -0.18f);
    glEnd();
    glColor3f(1.0f, 1.0f, 0.8f);
    drawText(-0.21f, -0.10f, "INSTRUCTIONS");

    // CREDITS Button (y: -0.46 to -0.26)
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.38f, 0.55f);
    glVertex2f(-0.30f, -0.26f); glVertex2f(0.30f, -0.26f);
    glColor3f(0.22f, 0.18f, 0.32f);
    glVertex2f(0.30f, -0.46f); glVertex2f(-0.30f, -0.46f);
    glEnd();
    glColor3f(1.0f, 1.0f, 0.8f);
    drawText(-0.14f, -0.37f, "CREDITS");

    // EXIT Button (y: -0.74 to -0.54)
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.38f, 0.55f);
    glVertex2f(-0.30f, -0.54f); glVertex2f(0.30f, -0.54f);
    glColor3f(0.22f, 0.18f, 0.32f);
    glVertex2f(0.30f, -0.74f); glVertex2f(-0.30f, -0.74f);
    glEnd();
    glColor3f(1.0f, 1.0f, 0.8f);
    drawText(-0.08f, -0.65f, "EXIT");
}


// ------------------------------------------------------------------------------------------------------------------
// Displays game instructions to the player
// Content: How to play, controls, win condition
// Design: Text displayed over transparent box on starry background
// ------------------------------------------------------------------------------------------------------------------
void drawInstructionsPage() {
    commonBackground();        // Draw starry background
    drawTransparentBox();      // Draw semi-transparent box for text readability

    glColor3f(1.0f, 1.0f, 0.8f); // Light cream text color

    drawText(-0.25f, 0.30f, "HOW TO PLAY");
    drawText(-0.65f, 0.15f, "- Move the basket using the keyboard.");
    drawText(-0.65f, 0.05f, "- Catch the falling stars to save them");
    drawText(-0.65f, -0.05f, "- Avoid bombs! Touching one ends the game.");
    drawText(-0.65f, -0.15f, "- Collect 5 stars to win.");
    drawText(-0.25f, -0.5f, "[ Click to return ]");
}


// ------------------------------------------------------------------------------------------------------------------
// Displays project information and team members
// Content: Project title, team members, course info, instructor
// Design: Text displayed over transparent box on starry background
// ------------------------------------------------------------------------------------------------------------------
void drawCreditsPage() {
    commonBackground();        // Draw starry background
    drawTransparentBox();      // Draw semi-transparent box for text readability

    glColor3f(1.0f, 1.0f, 0.8f); // Light cream text color

    drawText(-0.15f, 0.30f, "CREDITS");
    drawText(-0.55f, 0.15f, "Project Title: Save the Falling Stars");
    drawText(-0.55f, 0.03f, "Members: Lamyaa Mohammed, Sara Alharthi");
    drawText(-0.55f, -0.09f, "                  Ruwaina Alharbi, Taif Alqethami.");
    drawText(-0.55f, -0.21f, "Course: Computer Graphics");
    drawText(-0.55f, -0.33f, "Instructor: Dr. Hind Alsharif");
    drawText(-0.25f, -0.52f, "[ Click to return ]");
}


// ------------------------------------------------------------------------------------------------------------------
// Draws the background during active gameplay
// ------------------------------------------------------------------------------------------------------------------
void drawGameBackground() {

    // Draw gradient sky background
    // This creates a smooth color transition from bottom to top

    glBegin(GL_QUADS);

    // Bottom color (very dark blue)
    glColor3f(0.02f, 0.02f, 0.08f);
    glVertex2f(-1.0f, -1.0f);  // bottom-left corner
    glVertex2f(1.0f, -1.0f);   // bottom-right corner

    // Top color (deep purple)
    glColor3f(0.10f, 0.05f, 0.20f);
    glVertex2f(1.0f, 1.0f);    // top-right corner
    glVertex2f(-1.0f, 1.0f);   // top-left corner

    glEnd();

    // Draw ground line for basket position
    // This line represents the area where the basket is placed

    glColor3f(0.3f, 0.2f, 0.4f); // soft purple color
    glLineWidth(2.0f);           // make the line slightly thicker

    glBegin(GL_LINES);
    glVertex2f(-1.0f, -0.75f);   // left side of the ground
    glVertex2f(1.0f, -0.75f);    // right side of the ground
    glEnd();

    // Draw small decorative stars
    // Random small stars to make the background look alive

    glColor3f(1.0f, 0.95f, 0.7f); // light yellow color

    for (int i = 0; i < 15; i++) {

        // Generate random X position between -1 and 1
        float x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

        // Generate random Y position (upper part of the screen)
        float y = ((float)rand() / RAND_MAX) * 1.5f - 0.2f;

        // Draw star at random position
        drawStar(x, y);
    }

    // Draw the moon
    // Positioned at the top-right of the screen
    drawMoon(0.7f, 0.75f);
}


// ------------------------------------------------------------------------------------------------------------------
// Draws all moving game objects (star, bomb, basket)
// Called by: drawGameScreen during gameplay
// Objects use global position variables that are updated by animateGame
// ------------------------------------------------------------------------------------------------------------------
void drawGameObjects()
{
    drawFallingStar(starX, starY);  // Draw star at current position
    drawBomb(bombX, bombY);         // Draw bomb at current position
    drawBasket(0.0f, -0.7f);        // Draw basket (moves via basketX variable)
}


// ------------------------------------------------------------------------------------------------------------------
// Combines background and game objects for the gameplay screen
// ------------------------------------------------------------------------------------------------------------------
void drawGameScreen()
{
    drawGameBackground();  // Draw background
    drawGameObjects();     // Draw moving objects
}


// ------------------------------------------------------------------------------------------------------------------
// Displays the victory screen when player collects 5 stars
// Method: Loads and displays win.bmp texture as full-screen image
// ------------------------------------------------------------------------------------------------------------------
void drawWinScreen() {
    // Enable 2D texture mapping
    glEnable(GL_TEXTURE_2D);

    // Bind the win texture for rendering
    glBindTexture(GL_TEXTURE_2D, winTexture);

    // Draw a full-screen quad to display the texture
    glBegin(GL_QUADS);

    // Map texture coordinates (0,0 to 1,1) to screen coordinates (-1,-1 to 1,1)
    // This fills the entire window with the texture image

    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f); // Bottom left
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);  // Bottom right
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);   // Top right
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);  // Top left

    glEnd();

    // Disable texture mapping after drawing
    glDisable(GL_TEXTURE_2D);

    glFlush();
    glutSwapBuffers();
}


// ------------------------------------------------------------------------------------------------------------------
// Displays the game over screen when player hits a bomb
// Method: Loads and displays lose.bmp texture as full-screen image
// Texture source: Lab 6 LoadTexture function
// ------------------------------------------------------------------------------------------------------------------
void drawLoseScreen()
{
    // Enable 2D texture mapping
    glEnable(GL_TEXTURE_2D);

    // Bind the lose texture for rendering
    glBindTexture(GL_TEXTURE_2D, loseTexture);

    // Draw a full-screen quad to display the texture
    glBegin(GL_QUADS);

    // Map texture coordinates (0,0 to 1,1) to screen coordinates (-1,-1 to 1,1)
    // This fills the entire window with the texture image

    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f); // Bottom left
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);  // Bottom right
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);   // Top right
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);  // Top left

    glEnd();

    // Disable texture mapping after drawing
    glDisable(GL_TEXTURE_2D);

    glFlush();
    glutSwapBuffers();
}


// ------------------------------------------------------------------------------------------------------------------
// INPUT AND INTERACTION
// ------------------------------------------------------------------------------------------------------------------

// Processes mouse clicks for menu navigation and screen transitions
// Parameters: button (which button), state (up/down), x, y (pixel coordinates)
// Logic: Converts pixel coordinates to OpenGL coordinates, checks button bounds
// ------------------------------------------------------------------------------------------------------------------
void handleMouse(int button, int state, int x, int y)
{
    // Only process left mouse button clicks (ignore right button, scroll, etc.)
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Convert pixel coordinates (0-800, 0-600) to OpenGL coordinates (-1 to 1)
        float mouseX = (x / 400.0f) - 1.0f;  // Horizontal: 0-800 pixels -> -1 to 1
        float mouseY = (300.0f - y) / 300.0f; // Vertical: 0-600 pixels -> -1 to 1 (inverted)

        // If not on menu screen, any click returns to menu
        if (currentScreen != MENU)
        {
            currentScreen = MENU;
        }
        else // On menu screen, check which button was clicked
        {
            // All buttons are horizontally centered between x = -0.30 and x = 0.30
            if (mouseX >= -0.30f && mouseX <= 0.30f)
            {
                // START Button (y: 0.10 to 0.30)
                if (mouseY >= 0.10f && mouseY <= 0.30f)
                {
                    // Reset all game variables for a fresh start
                    gameOver = false;  // Allow game to run
                    score = 0;         // Reset score

                    // Reset star position
                    starY = 1.1f;      // Place above screen
                    starX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f; // Random X position

                    // Reset bomb position
                    bombY = 1.1f;      // Place above screen
                    bombX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f; // Random X position

                    basketX = 0.0f;    // Center the basket

                    currentScreen = GAME; // Switch to game screen
                }
                // INSTRUCTIONS Button (y: -0.18 to 0.02)
                else if (mouseY >= -0.18f && mouseY <= 0.02f)
                    currentScreen = INSTRUCTIONS;

                // CREDITS Button (y: -0.46 to -0.26)
                else if (mouseY >= -0.46f && mouseY <= -0.26f)
                    currentScreen = CREDITS;

                // EXIT Button (y: -0.74 to -0.54)
                else if (mouseY >= -0.74f && mouseY <= -0.54f)
                    exit(0); // Close the program
            }
        }

        glutPostRedisplay(); // Request screen redraw to show new screen
    }
}


// ------------------------------------------------------------------------------------------------------------------
// Handles arrow key presses to move the basket left and right
// Parameters: key (which special key), x, y (mouse position - not used)
// Keys: GLUT_KEY_LEFT (move left), GLUT_KEY_RIGHT (move right)
// ------------------------------------------------------------------------------------------------------------------
void specialKeys(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
        basketX -= 0.05f; // Move basket left by 0.05 units

    else if (key == GLUT_KEY_RIGHT)
        basketX += 0.05f; // Move basket right by 0.05 units

    glutPostRedisplay(); // Redraw screen to show new basket position
}


// ------------------------------------------------------------------------------------------------------------------
// Updates game state continuously during gameplay (game loop)
// Parameters: value (unused - required by GLUT timer callback)
// Frequency: Called every 16ms (~60 FPS)
// Updates: Star/bomb positions, collision detection, score, win/lose conditions
// ------------------------------------------------------------------------------------------------------------------
void animateGame(int value)
{
    // IMPORTANT: Only run game logic when on GAME screen
    if (currentScreen != GAME)
    {
        glutTimerFunc(16, animateGame, 0);  // Keep timer running but exit early
        return;
    }

    // Stop updating if game is over
    if (gameOver)
    {
        glutTimerFunc(16, animateGame, 0);  // Keep timer running but exit early
        return;
    }

    // === FALLING STAR LOGIC ===
    starY -= 0.015f; // Move star downward

    // Check if star reached basket height (collision zone)
    if (starY < -0.7f && starY > -0.9f)
    {
        // Check horizontal distance between star and basket center
        // fabs() calculates absolute value (distance without negative sign)
        if (fabs(starX - basketX) < 0.25f) // 0.25 = half of basket width
        {
            score++;         // Player caught the star! Increase score
            starY = 1.1f;    // Reset star above screen

            // Generate random X position between -0.8 and 0.8
            // rand() returns random int, divide by RAND_MAX to get 0.0-1.0
            // Multiply by 1.6 to get 0.0-1.6, subtract 0.8 to get -0.8 to 0.8
            starX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f;
        }
    }

    // If star fell off bottom of screen without being caught
    if (starY < -1.2f)
    {
        starY = 1.1f; // Reset to top
        starX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f; // New random position
    }

    // Check win condition: player collected 5 stars
    if (score >= 5)
    {
        currentScreen = WIN; // Switch to victory screen
    }

    // === BOMB LOGIC ===
    bombY -= 0.012f; // Move bomb downward (slightly slower than star)

    // Check if bomb reached basket height (collision zone)
    if (bombY < -0.7f && bombY > -0.9f)
    {
        // Check if bomb touched basket (smaller collision radius than star)
        if (fabs(bombX - basketX) < 0.20f)
        {
            gameOver = true;      // End the game
            currentScreen = LOSE; // Switch to game over screen
        }
    }

    // If bomb fell off bottom of screen
    if (bombY < -1.2f)
    {
        bombY = 1.1f; // Reset to top
        bombX = ((float)rand() / RAND_MAX) * 1.6f - 0.8f; // New random position
    }

    glutPostRedisplay();              // Redraw screen with updated positions
    glutTimerFunc(16, animateGame, 0); // Schedule next update in 16ms (60 FPS)
}
/*    Reference: rand() function - https://cplusplus.com/reference/cstdlib/rand/
                 fabs() function - https://cplusplus.com/reference/cmath/fabs/ */

                 // ------------------------------------------------------------------------------------------------------------------
                 // DISPLAY AND MAIN
                 // ------------------------------------------------------------------------------------------------------------------

                 // Main rendering function - decides which screen to draw based on currentScreen
                 // Called by: GLUT whenever screen needs to be redrawn (via glutPostRedisplay)
                 // ------------------------------------------------------------------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen (fills with black background)

    // Draw the appropriate screen based on current game state
    if (currentScreen == MENU)
        drawMainMenu();

    else if (currentScreen == GAME)
        drawGameScreen();

    else if (currentScreen == INSTRUCTIONS)
        drawInstructionsPage();

    else if (currentScreen == CREDITS)
        drawCreditsPage();

    else if (currentScreen == WIN)
        drawWinScreen();

    else if (currentScreen == LOSE)
        drawLoseScreen();

    glFlush(); // Force execution of OpenGL commands
}


// ------------------------------------------------------------------------------------------------------------------
// Initializes OpenGL settings and loads texture files
// Called by: main() before entering game loop
// Tasks: Set background color, load win/lose textures
// ------------------------------------------------------------------------------------------------------------------
void init()
{
    // ======= TEXTURE LOADING =======

    // Load win.bmp texture (800x600 pixels, 24-bit BMP)
    winTexture = LoadTexture(winImagePath, 800, 600);

    // Verify texture loaded successfully
    if (winTexture == -1 || winTexture == 0)
        cout << "Error: Failed to load win.bmp" << endl;
    else
        cout << "Win texture loaded successfully! ID: " << winTexture << endl;

    // Load lose.bmp texture (800x600 pixels, 24-bit BMP)
    loseTexture = LoadTexture(loseImagePath, 800, 600);

    // Verify texture loaded successfully
    if (loseTexture == -1 || loseTexture == 0)
        cout << "Error: Failed to load lose.bmp" << endl;
    else
        cout << "Lose texture loaded successfully! ID: " << loseTexture << endl;

    // ========================================================

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background to black
}


// ------------------------------------------------------------------------------------------------------------------
// Program entry point - initializes GLUT and starts the game loop
// Tasks: Create window, register callback functions, start event loop
// ------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    // Initialize GLUT library
    glutInit(&argc, argv);

    // Set display mode (single buffer, RGB color)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Set window dimensions (800x600 pixels)
    glutInitWindowSize(800, 600);

    // Set window position on screen (100 pixels from left, 100 from top)
    glutInitWindowPosition(100, 100);

    // Create window with title
    glutCreateWindow("Save the Falling Stars");

    // Call initialization function (load textures, set background)
    init();

    // Register callback functions
    glutDisplayFunc(display);       // Function to render graphics
    glutMouseFunc(handleMouse);     // Function to handle mouse clicks
    glutSpecialFunc(specialKeys);   // Function to handle special keys (arrows)
    glutTimerFunc(0, animateGame, 0); // Start animation timer (60 FPS)

    // Enter GLUT event loop (runs until window is closed)
    glutMainLoop();

    return 0;
}
