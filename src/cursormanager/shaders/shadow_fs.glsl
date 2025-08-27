#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec2 offset;
uniform float blurRadius;
uniform vec4 shadowColor;
uniform float alpha;

// Pre-calculated gaussian weights (sum to 1.0)
const float weights[3] = float[](0.4, 0.3, 0.2);

// Optimized shadow calculation with early exit
void main() {
    vec4 texColor = texture(image, TexCoords);
    
    // Early exit for fully transparent fragments
    if (texColor.a <= 0.0) {
        discard;
    }
    
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    vec2 shadowCoords = TexCoords - offset * texelSize;
    
    // Only calculate blur if the shadow would be visible
    vec4 shadowTex = texture(image, shadowCoords);
    if (shadowTex.a > 0.0) {
        FragColor = texColor;
        return;
    }
    
    // Optimized blur with fewer samples
    vec4 result = texture(image, TexCoords) * weights[0];
    
    // Sample in a plus pattern to reduce texture lookups
    for (int i = 1; i < 3; ++i) {
        float weight = weights[i];
        vec2 offset = vec2(texelSize.x * i * blurRadius, 0.0);
        result += texture(image, TexCoords + offset) * weight;
        result += texture(image, TexCoords - offset) * weight;
        
        offset = vec2(0.0, texelSize.y * i * blurRadius);
        result += texture(image, TexCoords + offset) * weight;
        result += texture(image, TexCoords - offset) * weight;
    }
    
    // Apply shadow with alpha blending
    result = mix(texColor, shadowColor, shadowColor.a * alpha * result.a);
    result.a = texColor.a; // Preserve original alpha
    
    FragColor = result;
}
