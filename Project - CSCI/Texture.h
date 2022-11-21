#pragma once
#include <vector>
#include <string>


class Texture 
{
public:
	void setupTexture(const char* texturePath);
	void loadSkybox(std::vector<std::string> faces);
	void bind(unsigned int slot) const;
	void unbind() const;

	void unbindSkybox() const;

	void bindSkybox(unsigned int slot) const;

private:
	unsigned int ID = 0;
	int Width = 0, Height = 0, BPP = 0;
};