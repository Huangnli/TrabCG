#version 130

in vec3 result;
out vec4 fragColor;                 //fragmento de saída com as cores com iluminação

void main(){

  //resulting light
  fragColor = vec4(result, 1.0f);

}

