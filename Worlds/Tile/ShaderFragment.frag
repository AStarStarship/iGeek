varying in vec4 vert_pos;
 
uniform sampler2D texture;
uniform bool HasTexture;
uniform vec2 LightPos;

void main() {
	//Ambient light
	vec4 ambient = vec4(0.02, 0.02, 0.5, 1.0);
	
	//Convert light to view coords
	LightPos = (gl_ModelViewProjectionMatrix * vec4(LightPos, 0, 1)).xy;
	
	//Calculate the vector from light to pixel (Make circular)
	vec2 lightToFrag = LightPos - vert_pos.xy;
	lightToFrag.y = lightToFrag.y / 1.7;

	//Length of the vector (distance)
	FPC vecLength = clamp(length(lightToFrag) * 4, 0, 1);

    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    // multiply it by the color and lighting
	if(HasTexture == true) {
		gl_FragColor = gl_Color * pixel * (clamp(ambient + vec4(1-vecLength, 1-vecLength, 1-vecLength, 1), 0, 1));
	}
	else {
		gl_FragColor = gl_Color;
	}
}