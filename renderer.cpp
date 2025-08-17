#include "mini_motorways_env.h"
#include <iostream>
#include <sstream>

// Vertex shader source
const std::string vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 color;

out vec3 vertexColor;

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    vertexColor = color;
}
)";

// Fragment shader source
const std::string fragment_shader_source = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

Renderer::Renderer() : shader_program(0), vao(0), vbo(0) {
    // Initialize color mappings
    tile_colors[TileType::EMPTY] = glm::vec3(0.1f, 0.1f, 0.1f);       // Dark gray
    tile_colors[TileType::HOUSE] = glm::vec3(0.8f, 0.2f, 0.2f);       // Red
    tile_colors[TileType::BUSINESS] = glm::vec3(0.2f, 0.2f, 0.8f);    // Blue
    tile_colors[TileType::ROAD] = glm::vec3(0.5f, 0.5f, 0.5f);        // Gray
    tile_colors[TileType::MOTORWAY] = glm::vec3(0.2f, 0.8f, 0.2f);    // Green
    tile_colors[TileType::BRIDGE] = glm::vec3(0.6f, 0.4f, 0.2f);      // Brown
    tile_colors[TileType::ROUNDABOUT] = glm::vec3(0.8f, 0.6f, 0.2f);  // Orange
    tile_colors[TileType::TRAFFIC_LIGHT] = glm::vec3(0.8f, 0.8f, 0.2f); // Yellow
    
    car_colors[CarColor::RED] = glm::vec3(1.0f, 0.0f, 0.0f);
    car_colors[CarColor::BLUE] = glm::vec3(0.0f, 0.0f, 1.0f);
    car_colors[CarColor::GREEN] = glm::vec3(0.0f, 1.0f, 0.0f);
    car_colors[CarColor::YELLOW] = glm::vec3(1.0f, 1.0f, 0.0f);
    car_colors[CarColor::PURPLE] = glm::vec3(1.0f, 0.0f, 1.0f);
    car_colors[CarColor::ORANGE] = glm::vec3(1.0f, 0.5f, 0.0f);
}

Renderer::~Renderer() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shader_program) glDeleteProgram(shader_program);
}

bool Renderer::initialize() {
    // Create and compile shaders
    shader_program = load_shader(vertex_shader_source, fragment_shader_source);
    if (shader_program == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }
    
    // Setup quad for rendering
    setup_quad();
    
    // Setup projection matrix (orthographic for 2D)
    projection = glm::ortho(0.0f, 25.0f, 20.0f, 0.0f, -1.0f, 1.0f);
    
    return true;
}

void Renderer::render_frame(const MiniMotorwaysEnvironment& env) {
    glUseProgram(shader_program);
    
    // Set projection matrix
    GLint proj_loc = glGetUniformLocation(shader_program, "projection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Render grid background
    render_grid(env.get_grid());
    
    // Render buildings
    render_buildings(env.get_buildings());
    
    // Render cars
    render_cars(env.get_cars());
    
    // Render UI
    render_ui(env.get_score(), env.get_step(), env.get_resources());
}

void Renderer::render_grid(const std::vector<std::vector<TileType>>& grid) {
    glBindVertexArray(vao);
    
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid[y].size(); x++) {
            TileType tile = grid[y][x];
            glm::vec3 color = tile_colors[tile];
            
            // Create model matrix for this tile
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(0.9f, 0.9f, 1.0f));  // Small gap between tiles
            
            // Set uniforms
            GLint model_loc = glGetUniformLocation(shader_program, "model");
            GLint color_loc = glGetUniformLocation(shader_program, "color");
            
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(color_loc, 1, glm::value_ptr(color));
            
            // Draw quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
}

void Renderer::render_buildings(const std::vector<Building>& buildings) {
    glBindVertexArray(vao);
    
    for (const auto& building : buildings) {
        glm::vec3 base_color = tile_colors[building.type];
        glm::vec3 accent_color = car_colors[building.color];
        
        // Mix base color with accent color
        glm::vec3 color = base_color * 0.7f + accent_color * 0.3f;
        
        // Create model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(building.position.x, building.position.y, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f));
        
        // Set uniforms
        GLint model_loc = glGetUniformLocation(shader_program, "model");
        GLint color_loc = glGetUniformLocation(shader_program, "color");
        
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(color_loc, 1, glm::value_ptr(color));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void Renderer::render_cars(const std::vector<std::shared_ptr<Car>>& cars) {
    glBindVertexArray(vao);
    
    for (const auto& car : cars) {
        if (car->completed) continue;
        
        glm::vec3 color = car_colors[car->color];
        
        // Use visual position for smooth movement
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(car->visual_x + 0.5f, car->visual_y + 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 1.0f));
        
        // Set uniforms
        GLint model_loc = glGetUniformLocation(shader_program, "model");
        GLint color_loc = glGetUniformLocation(shader_program, "color");
        
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(color_loc, 1, glm::value_ptr(color));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void Renderer::render_ui(int score, int step, const std::unordered_map<std::string, int>& resources) {
    // Simple UI rendering - resource bars on the right
    float ui_x = 21.0f;  // Right side of the grid
    float ui_y = 1.0f;
    
    glBindVertexArray(vao);
    
    // Resource bars
    std::vector<std::pair<std::string, glm::vec3>> resource_colors = {
        {"roads", glm::vec3(0.5f, 0.5f, 0.5f)},
        {"motorways", glm::vec3(0.2f, 0.8f, 0.2f)},
        {"bridges", glm::vec3(0.6f, 0.4f, 0.2f)},
        {"roundabouts", glm::vec3(0.8f, 0.6f, 0.2f)},
        {"traffic_lights", glm::vec3(0.8f, 0.8f, 0.2f)}
    };
    
    for (int i = 0; i < resource_colors.size(); i++) {
        const auto& [resource_name, color] = resource_colors[i];
        int count = resources.at(resource_name);
        
        for (int j = 0; j < count && j < 10; j++) {  // Max 10 bars per resource
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ui_x + j * 0.2f, ui_y + i * 0.5f, 0.0f));
            model = glm::scale(model, glm::vec3(0.15f, 0.3f, 1.0f));
            
            GLint model_loc = glGetUniformLocation(shader_program, "model");
            GLint color_loc = glGetUniformLocation(shader_program, "color");
            
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(color_loc, 1, glm::value_ptr(color));
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    
    // Score indicator
    int score_bars = std::min(score / 5, 20);  // 1 bar per 5 points, max 20 bars
    for (int i = 0; i < score_bars; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(ui_x + i * 0.1f, ui_y + 6.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.08f, 0.2f, 1.0f));
        
        GLint model_loc = glGetUniformLocation(shader_program, "model");
        GLint color_loc = glGetUniformLocation(shader_program, "color");
        
        glm::vec3 score_color = glm::vec3(0.0f, 1.0f, 0.0f);  // Green for score
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(color_loc, 1, glm::value_ptr(score_color));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

GLuint Renderer::load_shader(const std::string& vertex_src, const std::string& fragment_src) {
    // Compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v_src = vertex_src.c_str();
    glShaderSource(vertex_shader, 1, &v_src, nullptr);
    glCompileShader(vertex_shader);
    
    // Check vertex shader compilation
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "Vertex shader compilation failed: " << info_log << std::endl;
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_src = fragment_src.c_str();
    glShaderSource(fragment_shader, 1, &f_src, nullptr);
    glCompileShader(fragment_shader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "Fragment shader compilation failed: " << info_log << std::endl;
        return 0;
    }
    
    // Link shaders
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // Check linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

void Renderer::setup_quad() {
    // Quad vertices (2 triangles)
    float vertices[] = {
        // Triangle 1
        0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  // Bottom-right
        0.0f, 1.0f,  // Top-left
        
        // Triangle 2
        1.0f, 0.0f,  // Bottom-right
        1.0f, 1.0f,  // Top-right
        0.0f, 1.0f   // Top-left
    };
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}