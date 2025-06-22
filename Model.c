
#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 4
#define MAX_BONES 16



struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texCoords;
	unsigned int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Bone {
	char* name;
	mat4 offset;
};

struct Mesh {
	struct Vertex* vertices;
	unsigned int verticesCount;
	
	unsigned int* indices;
	unsigned int indexCount;
	
	unsigned int boneCount;
	char** boneNames;
	mat4* boneOffsets;
	
	unsigned int material;
	
	unsigned int VAO, VBO, EBO;
};

struct Model {
	struct Mesh* meshes;
	unsigned int meshCount;
	unsigned int* textures;
	unsigned int textureCount;
};

struct Object {
	struct Model model;
	vec3 pos;
	vec3 rot;
	vec3 scale;
};

struct Terrain {
	vec3* vertices;
	unsigned int vertexCount;
	unsigned int* indices;
	unsigned int faceCount;
};



struct Mesh processMesh(struct aiMesh* aiMesh, const struct aiScene* scene) {
printf("\n\n");
	unsigned int verticesCount = aiMesh->mNumVertices;
	struct Vertex* vertices = malloc(aiMesh->mNumVertices*sizeof(struct Vertex));
	
	unsigned int indexCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) { indexCount += aiMesh->mFaces[i].mNumIndices; }
	unsigned int* indices = malloc(indexCount*sizeof(unsigned int));
	
	// vertices
	for(unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
		struct Vertex vertex;
		vertex.position[0] = aiMesh->mVertices[i].x;
		vertex.position[1] = aiMesh->mVertices[i].y;
		vertex.position[2] = aiMesh->mVertices[i].z;
		vertex.normal[0] = aiMesh->mNormals[i].x;
		vertex.normal[1] = aiMesh->mNormals[i].y;
		vertex.normal[2] = aiMesh->mNormals[i].z;
		vertex.texCoords[0] = aiMesh->mTextureCoords[0][i].x;
		vertex.texCoords[1] = aiMesh->mTextureCoords[0][i].y;
		for (int ii = 0; ii < MAX_BONE_WEIGHTS; ii++) {
			vertex.m_BoneIDs[ii] = -1;
			vertex.m_Weights[ii] = 0.0f;
		}
		vertices[i] = vertex;
	}
	
	// indices
	indexCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
		for(unsigned int j = 0; j < aiMesh->mFaces[i].mNumIndices; j++) {
			indices[indexCount] = aiMesh->mFaces[i].mIndices[j];
			indexCount++;
		}
	}
	
	// bones
	char* boneIndicesNames[aiMesh->mNumBones];
	unsigned int boneIndices[aiMesh->mNumBones];
	unsigned int boneCount = 0;
	for(unsigned int i = 0; i < aiMesh->mNumBones; i++) {
		char* boneName = aiMesh->mBones[i]->mName.data;
		unsigned int ii;
		for(ii = 0; ii < boneCount; ii++) if(strcmp(boneName, boneIndicesNames[ii]) == 0) break;
		if(ii >= boneCount) {
			boneIndicesNames[boneCount] = boneName;
			boneIndices[boneCount] = boneCount;
// printf("%d %s\n", boneCount, boneName);
			boneCount++;
		}
	}
	
	char** boneNames = malloc(boneCount);
	mat4* boneOffsets = malloc(sizeof(mat4) * boneCount);
	
	for(unsigned int i = 0; i < boneCount; i++) {
		boneNames[i] = malloc(aiMesh->mBones[boneIndices[i]]->mName.length+1);
		strcpy(boneNames[i], aiMesh->mBones[boneIndices[i]]->mName.data);
		
		boneOffsets[i][0][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a1;
		boneOffsets[i][0][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b1;
		boneOffsets[i][0][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c1;
		boneOffsets[i][0][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d1;
		boneOffsets[i][1][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a2;
		boneOffsets[i][1][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b2;
		boneOffsets[i][1][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c2;
		boneOffsets[i][1][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d2;
		boneOffsets[i][2][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a3;
		boneOffsets[i][2][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b3;
		boneOffsets[i][2][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c3;
		boneOffsets[i][2][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d3;
		boneOffsets[i][3][0] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.a4;
		boneOffsets[i][3][1] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.b4;
		boneOffsets[i][3][2] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.c4;
		boneOffsets[i][3][3] = aiMesh->mBones[boneIndices[i]]->mOffsetMatrix.d4;
printf("%d %s \n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", i, boneNames[i],
 boneOffsets[i][0][0], boneOffsets[i][1][0], boneOffsets[i][2][0], boneOffsets[i][3][0],
 boneOffsets[i][0][1], boneOffsets[i][1][1], boneOffsets[i][2][1], boneOffsets[i][3][1],
 boneOffsets[i][0][2], boneOffsets[i][1][2], boneOffsets[i][2][2], boneOffsets[i][3][2],
 boneOffsets[i][0][3], boneOffsets[i][1][3], boneOffsets[i][2][3], boneOffsets[i][3][3]);
		
		for(unsigned int ii = 0; ii < aiMesh->mBones[boneIndices[i]]->mNumWeights; ii++) {
			struct aiVertexWeight weight = aiMesh->mBones[boneIndices[i]]->mWeights[ii];
			for(unsigned int iii = 0; iii < MAX_BONE_INFLUENCE; iii++) {
				if(vertices[weight.mVertexId].m_BoneIDs[iii] == -1 && weight.mWeight != 0.0f) {
printf("%d", i);
if(weight.mWeight != 1.0f) printf("!");
printf(" ");
					vertices[weight.mVertexId].m_BoneIDs[iii] = i;
					vertices[weight.mVertexId].m_Weights[iii] = weight.mWeight;
					break;
				}
			}
		}
printf("\n");
	}
	
	
	
	// // bone transformation
	// for(unsigned int i = 0; i < verticesCount; i++) {
	// 	vec4 totalPosition = { 0.0f, 0.0f, 0.0f, 0.0f };
	// 	vec3 totalNormal = { 0.0f, 0.0f, 0.0f };
	// 	vec4 position = { vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f };
	// 	vec3 normal = { vertices[i].normal[0], vertices[i].normal[1], vertices[i].normal[2] };
		
	// 	bool hasBones = false;
	// 	for(int ii = 0; ii < MAX_BONE_INFLUENCE; ii++) {
	// 		if(vertices[i].m_BoneIDs[ii] == -1) continue;
	// 		hasBones = true;
	// 		if(vertices[i].m_BoneIDs[ii] >= MAX_BONES) {
	// 			totalPosition[0] = position[0];
	// 			totalPosition[1] = position[1];
	// 			totalPosition[2] = position[2];
	// 			totalPosition[3] = position[3];
	// 			totalNormal[0] = normal[0];
	// 			totalNormal[1] = normal[1];
	// 			totalNormal[2] = normal[2];
	// 			break;
	// 		}
			
	// 		vec4 localPosition;
	// 		glm_mat4_mulv(boneOffsets[vertices[i].m_BoneIDs[ii]], position, localPosition);
	// 		localPosition[0] *= vertices[i].m_Weights[ii];
	// 		localPosition[1] *= vertices[i].m_Weights[ii];
	// 		localPosition[2] *= vertices[i].m_Weights[ii];
	// 		localPosition[3] *= vertices[i].m_Weights[ii];
	// 		totalPosition[0] += localPosition[0];
	// 		totalPosition[1] += localPosition[1];
	// 		totalPosition[2] += localPosition[2];
	// 		totalPosition[3] += localPosition[3];
			
	// 		vec3 localNormal;
	// 		glm_mat4_mulv3(boneOffsets[vertices[i].m_BoneIDs[ii]], normal, 3, localNormal);
	// 		localNormal[0] *= vertices[i].m_Weights[ii];
	// 		localNormal[1] *= vertices[i].m_Weights[ii];
	// 		localNormal[2] *= vertices[i].m_Weights[ii];
	// 		totalNormal[0] += localNormal[0];
	// 		totalNormal[1] += localNormal[1];
	// 		totalNormal[2] += localNormal[2];
	// 	}
		
	// 	if(!hasBones) {
	// 		totalPosition[0] = position[0];
	// 		totalPosition[1] = position[1];
	// 		totalPosition[2] = position[2];
	// 		totalPosition[3] = position[3];
	// 		totalNormal[0] = normal[0];
	// 		totalNormal[1] = normal[1];
	// 		totalNormal[2] = normal[2];
	// 	}
		
	// 	vertices[i].position[0] = totalPosition[0];
	// 	vertices[i].position[1] = totalPosition[1];
	// 	vertices[i].position[2] = totalPosition[2];
	// 	vertices[i].normal[0] = totalNormal[0];
	// 	vertices[i].normal[1] = totalNormal[1];
	// 	vertices[i].normal[2] = totalNormal[2];
	// }
	
	
	
	struct Mesh finalMesh = {
		.vertices = vertices,
		.verticesCount = verticesCount,
		.indices = indices,
		.indexCount = indexCount,
		.material = aiMesh->mMaterialIndex,
		.boneCount = boneCount,
		.boneOffsets = boneOffsets,
		.boneNames = boneNames
	};
// printf("\n");
// for(unsigned int i = 0; i < verticesCount; i++) {
// 	for(unsigned int ii = 0; ii < MAX_BONE_INFLUENCE; ii++) {
// 		printf("%d %f ", vertices[i].m_BoneIDs[ii], vertices[i].m_Weights[ii]);
// 	}
// 	printf("\n");
// }
// printf("\n");
	
	glGenVertexArrays(1, &finalMesh.VAO);
	glGenBuffers(1, &finalMesh.VBO);
	glGenBuffers(1, &finalMesh.EBO);
	
	glBindVertexArray(finalMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, finalMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, finalMesh.verticesCount*sizeof(*finalMesh.vertices), &finalMesh.vertices[0].position[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, finalMesh.indexCount*sizeof(*finalMesh.indices), &finalMesh.indices[0], GL_STATIC_DRAW);
	
	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)0);
	// normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, normal));
	// texture attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(*finalMesh.vertices), (void*)offsetof(struct Vertex, texCoords));
	// bone id attribute
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_INT, sizeof(struct Vertex), (void*)offsetof(struct Vertex, m_BoneIDs));
	// bone weight attribute
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, m_Weights));
	
	glBindVertexArray(0);
	
	return finalMesh;
}

unsigned int meshCount;

void processNode(struct aiNode* node, struct Mesh* meshes, const struct aiScene* scene) {
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		struct aiMesh *aiMesh = scene->mMeshes[node->mMeshes[i]]; 
		meshes[meshCount] = processMesh(aiMesh, scene);
		meshCount++;
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], meshes, scene);
	}
}

unsigned int countMeshes(struct aiNode* node) {
	unsigned int meshCount = node->mNumMeshes;
	for(unsigned int i = 0; i < node->mNumChildren; i++) meshCount += countMeshes(node->mChildren[i]);
	return meshCount;
}

struct Terrain createTerrain(char* modelPath) {
	struct Terrain terrain;
	
	// assiimp scene
	const struct aiScene* scene = aiImportFile( modelPath,
	aiProcess_CalcTangentSpace       |
	aiProcess_Triangulate            |
	aiProcess_JoinIdenticalVertices  |
	aiProcess_SortByPType);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf(aiGetErrorString());
		return terrain;
	}
	
	terrain.vertexCount = 0;
	terrain.faceCount = 0;
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		terrain.vertexCount += scene->mMeshes[i]->mNumVertices;
		terrain.faceCount += scene->mMeshes[i]->mNumFaces;
	}
	
	terrain.vertices = malloc(terrain.vertexCount*sizeof(vec3));
	terrain.indices  = malloc(terrain.faceCount*3*sizeof(unsigned int));
	
	unsigned int vertexIndex = 0;
	unsigned int indexIndex  = 0;
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		for(unsigned int ii = 0; ii < scene->mMeshes[i]->mNumVertices; ii++) {
			terrain.vertices[vertexIndex][0] = scene->mMeshes[i]->mVertices[ii].x;
			terrain.vertices[vertexIndex][1] = scene->mMeshes[i]->mVertices[ii].y;
			terrain.vertices[vertexIndex][2] = scene->mMeshes[i]->mVertices[ii].z;
			vertexIndex++;
		}
		for(unsigned int ii = 0; ii < scene->mMeshes[i]->mNumFaces; ii++) {
			terrain.indices[indexIndex*3]   = scene->mMeshes[i]->mFaces[ii].mIndices[0];
			terrain.indices[indexIndex*3+1] = scene->mMeshes[i]->mFaces[ii].mIndices[1];
			terrain.indices[indexIndex*3+2] = scene->mMeshes[i]->mFaces[ii].mIndices[2];
			indexIndex++;
		}
	}
	
	aiReleaseImport(scene);
	
	return terrain;
}

struct Model loadModel(char* modelPath) {
	struct Model model;
	
	// assimp scene
	const struct aiScene* scene = aiImportFile( modelPath,
	 aiProcess_CalcTangentSpace       |
	 aiProcess_Triangulate            |
	 aiProcess_JoinIdenticalVertices  |
	 aiProcess_SortByPType);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf(aiGetErrorString());
		return model;
	}
	
	// meshes
	model.meshCount = countMeshes(scene->mRootNode);
	model.meshes = malloc(model.meshCount*sizeof(struct Mesh));
	meshCount = 0;
	processNode(scene->mRootNode, model.meshes, scene);
	
	// textures
	model.textures = malloc(scene->mNumMaterials*sizeof(unsigned int));
	glGenTextures(scene->mNumMaterials, model.textures);
	char* texPath = (char*)malloc(256*sizeof(char));
	unsigned int texPathLength = strrchr(modelPath, '/')+1 - modelPath;
	strncpy(texPath, modelPath, texPathLength);
	for(unsigned int i = 1; i < scene->mNumMaterials; i++) {	
		struct aiString texStr;
		aiGetMaterialTexture(scene->mMaterials[i], aiTextureType_DIFFUSE, 0, &texStr, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0);
		strcpy(&texPath[texPathLength], texStr.data);
		int texWidth, texHeight, nrChannels;
		glBindTexture(GL_TEXTURE_2D, model.textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		unsigned char *texData = stbi_load(texPath, &texWidth, &texHeight, &nrChannels, 0);
		if(texData) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			printf("Failed to load texture \"%s\".\n", texPath);
		}
		stbi_image_free(texData);
	}
	free(texPath);
	
	aiReleaseImport(scene);
	
	return model;
}

struct Object createObject(struct Model model) {
	struct Object object;
	
	object.model = model;
	object.pos[0] = 0.0f;
	object.pos[1] = 0.0f;
	object.pos[2] = 0.0f;
	object.rot[0] = 0.0f;
	object.rot[1] = 0.0f;
	object.rot[2] = 0.0f;
	object.scale[0] = 1.0f;
	object.scale[1] = 1.0f;
	object.scale[2] = 1.0f;
	
	return object;
}
