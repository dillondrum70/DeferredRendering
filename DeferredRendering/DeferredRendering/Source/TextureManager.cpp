#include "TextureManager.h"

TextureManager::TextureManager()
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		textures[i] = Texture(GL_TEXTURE0 + i);
	}
}

Texture TextureManager::AddTexture(const char* texFilePath)
{
	textures[textureCount] = Texture(GL_TEXTURE0 + textureCount);
	textures[textureCount].CreateTexture(texFilePath);

	glActiveTexture(textures[textureCount].texNumber);
	glBindTexture(GL_TEXTURE_2D, textures[textureCount].GetTexture());

	textureCount++;

	return textures[textureCount - 1];
}

Texture TextureManager::AddNormalMap(const char* normalFilePath)
{
	normals[normalCount] = Texture(GL_TEXTURE0 + MAX_TEXTURES + normalCount);
	normals[normalCount].CreateTexture(normalFilePath);

	glActiveTexture(normals[normalCount].texNumber);
	glBindTexture(GL_TEXTURE_2D, normals[normalCount].GetTexture());

	normalCount++;

	return normals[normalCount - 1];
}