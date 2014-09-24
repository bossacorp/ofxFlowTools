
#pragma once

#include "ofMain.h"


#define STRINGIFY(A) #A
#define GLSL(version, shader)  "#version " #version "\n" #shader


/*
 
 ofLogVerbose("init ");
	if (isProgrammableRenderer)
		glThree();
	else
		glTwo();
}

protected:
void glTwo() {
	
}

void glThree() {
 
 in vec2 texCoordVarying;
 out vec4 fragColor;
 
 texture
 
 shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
 shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
 shader.bindDefaults();
 shader.linkProgram();
}
 
public:
 
*/

namespace flowTools {
	
	class ftShader{
	public:
		ftShader() {
			
			if(ofGetGLProgrammableRenderer())
				isProgrammableRenderer = true;
			else
				isProgrammableRenderer = false;
				
			if (isProgrammableRenderer) {
				vertexShader = GLSL(150,
								uniform mat4 modelViewProjectionMatrix;
								uniform mat4 textureMatrix;
								
								in vec4 position;
								in vec2	texcoord;
								in vec4	color;
								
								out vec2 texCoordVarying;
								out vec4 colorVarying;
								
								void main()
								{
									colorVarying = color;
									texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
									gl_Position = modelViewProjectionMatrix * position;
								}
								);
			
			fragmentShader = GLSL(150,
								  out vec4 fragColor;
								
								  void main()
								  {
									  fragColor = vec4(0.0,1.0,0.0,1.0);
								  }
								  );
			}
			else {
				
				vertexShader = GLSL(120,
									void main() {
										gl_Position = gl_Vertex;
										gl_FrontColor = gl_Color;
									}
									);
										 
				
				fragmentShader = GLSL(120,
									  void main()
									  {
										  gl_FragColor = vec4(0.0,1.0,0.0,1.0);
									  }
									  );
			}
			
			quad.getVertices().resize(4);
			quad.getTexCoords().resize(4);
			quad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		}
		
		~ftShader() {shader.unload();};
		
	protected:
		
		void renderFrame(float _width, float _height){
			quad.setVertex(0, ofVec3f(0,0,0));
			quad.setVertex(1, ofVec3f(_width,0,0));
			quad.setVertex(2, ofVec3f(_width,_height,0));
			quad.setVertex(3, ofVec3f(0,_height,0));
			
			quad.setTexCoord(0, ofVec2f(0,0));
			quad.setTexCoord(1, ofVec2f(_width,0));
			quad.setTexCoord(2, ofVec2f(_width,_height));
			quad.setTexCoord(3, ofVec2f(0,_height));
			
			quad.draw(); 
		}
		
		void renderNormalizedFrame(float _width, float _height){
			quad.setVertex(0, ofVec3f(0,0,0));
			quad.setVertex(1, ofVec3f(_width,0,0));
			quad.setVertex(2, ofVec3f(_width,_height,0));
			quad.setVertex(3, ofVec3f(0,_height,0));
			
			quad.setTexCoord(0, ofVec2f(0,0));
			quad.setTexCoord(1, ofVec2f(1,0));
			quad.setTexCoord(2, ofVec2f(1,1));
			quad.setTexCoord(3, ofVec2f(0,1));
			
			quad.draw();
		}
		
		
		ofMesh quad;
		
		ofShader shader;
		string fragmentShader;
		string vertexShader;
		
		bool isProgrammableRenderer = false;
	};
	
}