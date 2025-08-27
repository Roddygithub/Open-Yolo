#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform float glowSize;
uniform float glowIntensity;
uniform vec4 glowColor;
uniform float alpha;

// Optimized weights for glow effect
const float weights[3] = float[](0.5, 0.3, 0.2);

void main() {
    vec4 texColor = texture(image, TexCoords);
    
    // Early exit for fully transparent fragments
    if (texColor.a <= 0.0) {
        discard;
    }
    
    // Only apply glow if intensity is sufficient
    if (glowIntensity <= 0.01 || glowSize <= 0.01) {
        FragColor = texColor;
        return;
    }
    
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));
    vec4 result = vec4(0.0);
    
    // Optimized sampling with fewer samples
    for (int i = 0; i < 3; ++i) {
        float weight = weights[i];
        float offset = float(i + 1) * glowSize * 0.02;
        
        // Sample in 4 directions (up, down, left, right)
        vec2 offsets[4] = vec2[](
            vec2(offset, 0.0),
            vec2(-offset, 0.0),
            vec2(0.0, offset),
            vec2(0.0, -offset)
        );
        
        for (int j = 0; j < 4; ++j) {
            vec2 samplePos = TexCoords + offsets[j] * texelSize;
            if (all(greaterThanEqual(samplePos, vec2(0.0))) && 
                all(lessThanEqual(samplePos, vec2(1.0)))) {
                result += texture(image, samplePos) * weight;
            }
        }
    }
    
    // Apply glow with intensity and color
    vec4 glow = glowColor * glowIntensity * result.a * alpha;
    
    // Blend with original color
    FragColor = mix(texColor, glow, glow.a);
    FragColor.a = texColor.a; // Preserve original alpha
}
