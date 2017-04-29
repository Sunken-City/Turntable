#version 410 core

uniform mat4 gModel;

//Also good for a block
uniform mat4 gView;
uniform mat4 gProj;

//Uniform Blocks are SUPER USEFUL for this!
uniform mat4 gBoneMatrices[200]; //max supported bones (inverse_initial * current)

in vec3 inPosition;
in vec3 inNormal;

//When you pass this up, pass using glVertexAttribIPointer
in ivec4 inBoneIndices;
in vec4 inBoneWeights;

out vec3 passPosition;
out vec4 passColor;
out vec3 passNormal;

void main(void)
{
    mat4 bone0 = gBoneMatrices[inBoneIndices.x];
    mat4 bone1 = gBoneMatrices[inBoneIndices.y];
    mat4 bone2 = gBoneMatrices[inBoneIndices.z];
    mat4 bone3 = gBoneMatrices[inBoneIndices.w];

    mat4 boneTransform = inBoneWeights.x * bone0
                       + inBoneWeights.y * bone1
                       + inBoneWeights.z * bone2
                       + inBoneWeights.w * bone3;
    mat4 modelToWorld = boneTransform * gModel;
    passPosition = (vec4(inPosition, 1.0f) * modelToWorld).xyz;
    passNormal = (vec4(inNormal, 1.0f) * modelToWorld).xyz;
    //Tangent, bitangent...

    //remove debuging by removing this line.
    passColor = vec4(inBoneWeights.xyz, 1.0f);

    //Pass position over
    gl_Position = vec4(inPosition, 1.0f) * modelToWorld * gView * gProj;
}