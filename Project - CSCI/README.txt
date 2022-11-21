Instead of having a shader method in the main file, you can just the Shader.cpp methods to send stuff to the vertex and fragment shader.
For example, sending a Matrix4f called matrixName from main to our shader
shader.setMat4("name of variable in the shader", matrixName);


I follow the convention for naming
vaoOBJECTNAME, vboOBJECTNAME, eboOBJECTNAME

Transformation, scaling, projection, viewing matrices are all defined under the function void matrix()
You can obviously introduce your own, if you comment mine out use /* */  and with the note COMMENTED OUT (so it's to find)

There are two shaders, one for all the objects and one for the skybox. You can consider addign a shader for the skybox

Currently, you can move the WHOLE screen with the mouse (but will change this later since we don't need this and I wanted to try it out)

If you make new changes, add the comments to the github commit.
