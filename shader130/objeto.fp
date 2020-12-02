#version 130
//out vec3 color;                   //cor do objeto

//uniform vetorLight[i]->lightPosition;

//uniform vec3 lightColor;          //for ambient
uniform float Ka;

in vec3 normal;                     //for diffuse
in vec3 fragPosition;
uniform float Kd;
uniform vec3 lightPosition;

uniform vec3 viewerPosition;        //for specular
uniform float Ks;                 

uniform vec3 outColor;              //for resulting light
out vec4 fragColor;                 //fragmento de saída com as cores com iluminação


void main(){
  vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);  

  //ambient calculating
  vec3 ambient = Ka*lightColor;

  //diffuse calculating
  vec3 norm = normalize(normal);
  vec3 lightDirection = normalize(lightPosition - fragPosition);    //vector L
  float diff = max(dot(norm, lightDirection), 0.0f);
  vec3 diffuse = Kd * diff * lightColor;

  //specular calculating
  vec3 vDirection = normalize(viewerPosition - fragPosition);       //vector V
  vec3 reflection = reflect(-lightDirection, norm);                 //vector R
  float specComp = pow(max(dot(vDirection, reflection), 0.0f), 32);
  vec3 specular = Ks * specComp * lightColor;                       //Is


  //resulting light
  vec3 result = (ambient + diffuse + specular) * outColor;
  fragColor = vec4(result, 1.0f);

  //vec3 result = ambient * outColor;
  //vec3 result = (ambient + diffuse) * outColor;
  //fragColor = vec4(outColor, 1.0f);
  //color = outColor;
}
