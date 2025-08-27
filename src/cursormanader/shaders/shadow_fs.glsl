#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec2 offset;
uniform float blurRadius;
uniform vec4 shadowColor;
uniform float alpha;

void main() {
    vec2 texSize = textureSize(image, 0).xy;
    vec2 texelSize = 1.0 / texSize;
    
    vec4 result = vec4(0.0);
    float total = 0.0;
    
    // Simple blur kernel for shadow
    for (float x = -blurRadius; x <= blurRadius; x++) {
        for (float y = -blurRadius; y <= blurRadius; y++) {
            vec2 samplePos = TexCoords + vec2(x, y) * texelSize * blurRadius * 0.2;
            float weight = 1.0 - length(vec2(x, y)) / (blurRadius + 1.0);
            weight = max(0.0, weight);
            
            vec4 tex = texture(image, samplePos);
            float alphaMask = tex.a * weight;
            
            result += vec4(shadowColor.rgb, shadowColor.a * alphaMask);
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
