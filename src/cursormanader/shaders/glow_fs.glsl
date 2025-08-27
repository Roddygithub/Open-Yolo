#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform float glowSize;
uniform float glowIntensity;
uniform vec4 glowColor;
uniform float alpha;

void main() {
    vec2 texSize = textureSize(image, 0).xy;
    vec2 texelSize = 1.0 / texSize;
    
    vec4 result = vec4(0.0);
    float total = 0.0;
    
    // Gaussian blur for glow effect
    int samples = int(glowSize * 2.0);
    float sigma = glowSize * 0.5;
    
    for (int x = -samples; x <= samples; x++) {
        for (int y = -samples; y <= samples; y++) {
            float dist = length(vec2(x, y));
            if (dist > samples) continue;
            
            float weight = exp(-(dist * dist) / (2.0 * sigma * sigma)) / (2.0 * 3.14159 * sigma * sigma);
            vec2 samplePos = TexCoords + vec2(x, y) * texelSize * 2.0;
            
            vec4 tex = texture(image, samplePos);
            result += vec4(glowColor.rgb, glowColor.a * weight * glowIntensity * tex.a);
            total += weight;
        }
    }
    
    if (total > 0.0) {
        result /= total;
    }
    
    // Apply alpha
    result.a *= alpha;
    FragColor = result;
}
