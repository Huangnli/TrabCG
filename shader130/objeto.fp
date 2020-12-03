#version 130

uniform int nLuzes;
uniform vec3 lights[10];

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

vec3 calcIntensityLight(vec3 light, vec3 norm, vec3 fragPos, vec3 viewDir);


void main(){
  vec3 lightColor = vec3(1.0f, 1.0f, 1.0f); 
  
  vec3 norm = normalize(normal);
  vec3 viewDirection = normalize(viewerPosition - fragPosition);       //vector V

  //ambient calc
  vec3 ambient = Ka*lightColor;

  vec3 finalResult = ambient;
  
  for(int i = 0; i < nLuzes; i++){
      finalResult += calcIntensityLight(lights[i], norm, fragPosition, viewDirection);
  }

  //resulting light
  finalResult *= outColor;
  fragColor = vec4(finalResult, 1.0f);

}

//calcula a intensidade pontual de uma fonte de luz
vec3 calcIntensityLight(vec3 light, vec3 norm, vec3 fragPos, vec3 viewDir){
    
    //diffuse calc
    vec3 lightDir = normalize(light - fragPos);  
    float diff = max(dot(norm, lightDir), 0.0f);

    //specular calc
    vec3 reflection = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflection), 0.0f), 32);

    //attenuation calc
    float distance = length(lightPosition - fragPosition);
    float fatt = 1/(1 + distance + 0.25*(pow(distance, 2) ) );

    //combinação das reflexoes para uma fonte - ambiente nao entra
    vec3 diffuse = Kd * diff * vec3(1.0f, 1.0f, 1.0f);
    vec3 specular = Ks * spec * vec3(1.0f, 1.0f, 1.0f);

    return fatt*(diffuse + specular);

}