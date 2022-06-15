#version 330 core
uniform sampler2D uGNormalWorld;
uniform sampler2D uGPosition;
uniform sampler2D uBeauty;
uniform int Width;
uniform int Height;
// sigma parameter
float m_sigmaPlane = 0.1f;
float m_sigmaColor = 0.3f;
float m_sigmaNormal = 0.3f;
float m_sigmaCoord = 32.0f;
const int kernelRadius = 8;


vec2 Getuv(float x,float y){
    return vec2(x/Width,y/Height);
}
float square(float a){
    return a*a;
}

void main(){
    float sum_of_weight = 0.0f;
    vec3 sum_of_weight_values=vec3(0.0);
    float x=gl_FragCoord.x;
    float y=gl_FragCoord.y;
    vec3 tem;
    for (float ky=y-kernelRadius/2;ky<=y+kernelRadius/2;ky++){
        if (ky<0) continue;
        for(float kx=x-kernelRadius/2;kx<=x+kernelRadius/2;kx++){
            if(kx<0) continue;
            float e_p=(square(ky-y)+square(kx-x))/(2*square(m_sigmaCoord));
            float e_c=square(distance(texture2D(uBeauty,Getuv(x,y)),texture2D(uBeauty,Getuv(kx,ky))))/(2*square(m_sigmaColor));
            float e_n;
            vec3 N=normalize((texture2D(uGNormalWorld,Getuv(x,y)).xyz-vec3(0.5))*2);
            vec3 kN=normalize((texture2D(uGNormalWorld,Getuv(kx,ky)).xyz-vec3(0.5))*2);
            if(length(N)==0.0||length(kN)==0.0){
                e_n=0.0;
            }
            else{
                float D_n=acos(dot(N,kN));
                if(dot(N,kN)>1-1e3) D_n=0;
                e_n=D_n*D_n/(2*m_sigmaNormal*m_sigmaNormal);
            }
            vec3 I=texture2D(uGPosition,Getuv(x,y)).xyz;
            vec3 J=texture2D(uGPosition,Getuv(kx,ky)).xyz;
            float D_p;
            if(distance(I,J)==0.0) D_p=0;
            else D_p=dot(N,(I-J)/distance(I,J));
            float e_d=D_p*D_p/(2*square(m_sigmaPlane));
            float weight=exp(-e_p-e_c-e_n-e_d);
            sum_of_weight+=weight;
            sum_of_weight_values+=texture2D(uBeauty,Getuv(kx,ky)).xyz*weight;
        }
    }
    gl_FragColor=vec4(sum_of_weight_values/sum_of_weight,1.0);
}