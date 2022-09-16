#pragma once
/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert -> edited by Jarno De Cooman
/*=============================================================================*/
// EOBJParser.h: most basic OBJParser!
/*=============================================================================*/
#include <string>
#include <fstream>
#include <vector>
#include "EMath.h"
#include "Mesh.h"

namespace Elite
{
	//Just parses vertices and indices
	static bool ParseOBJ(const std::string& filename, std::vector<Mesh::Base_Vertex>&
		vertices, std::vector<uint32_t>& indices)
	{
		std::ifstream file(filename);
		if (!file)
			return false;
		std::vector<FPoint4> positions;
		std::vector<FVector3> normals;
		std::vector<FVector2> UVs;

		int counter{};

		//--- Clear the vectors ---
		vertices.clear();
		indices.clear();

		std::string sCommand;
		// start a while iteration ending when the end of file is reached(ios::eof)
		while (!file.eof())
		{
			file >> sCommand;

			//read the first word of the string, use the >> operator (istream::operator>>)file >> sCommand;
			//use conditional statements to process the different commands
			if (sCommand == "#")
			{
				// Ignore Comment
			}
			else if (sCommand == "v")
			{
				//Vertex
				float x, y, z;
				file >> x >> y >> z;
				positions.push_back(FPoint4(x, y, z,1.f));
			}
			else if (sCommand == "vt")
			{
				// Vertex TexCoord
				float u, v;
				file >> u >> v;
				UVs.push_back(FVector2(u, 1 - v));
			}
			else if (sCommand == "vn")
			{
				// Vertex Normal
				float x, y, z;
				file >> x >> y >> z;
				normals.push_back(FVector3(x, y, z));
			}
			else if (sCommand == "f")
			{
				//if a face is read:
				//construct the 3 vertices, add them to the vertex array
				//add three indices to the index array
				//add the material index as attibute to the attribute array
				//
				// Faces or triangles
				size_t iPosition, iTexCoord, iNormal;
				for (size_t iFace = 0; iFace < 3; iFace++)
				{
					// OBJ format uses 1-based arrays
					Mesh::Base_Vertex vertex{  };
					file >> iPosition;
					vertex.Position = positions[iPosition - 1];
					if ('/' == file.peek())//is next in buffer ==  '/' ?
					{
						file.ignore();//read and ignore one element ('/')
						if ('/' != file.peek())
						{
							// Optional texture coordinate
							file >> iTexCoord;
							vertex.Uv = UVs[iTexCoord - 1];
						}
						if ('/' == file.peek())
						{
							file.ignore();
							// Optional vertex normal
							file >> iNormal;
							vertex.Normal = normals[iNormal - 1];
						}
					}

					////--- don't make duplicates ---
					//const auto it = find_if(vertices.begin(), vertices.end(),
					//	[&](const Mesh::Base_Vertex& v)
					//	{
					//		return v.Position == vertex.Position && v.Uv == vertex.Uv;
					//	});
					//
					//if (it == vertices.end())
					//{
					//	vertices.push_back(vertex);
					//	indices.push_back(vertices.size() - 1);
					//}
					//else
					//	indices.push_back(it - vertices.begin());

					vertices.push_back(vertex);
					indices.push_back(counter);
					++counter;
				}
			}
			//read till end of line and ignore all remaining chars
			file.ignore(1000, '\n');

		}

		//--- Set Tangent of every vertex---
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			const int index0 = indices[i];
			const int index1 = indices[i + 1];
			const int index2 = indices[i + 2];

			const FPoint3& p0 = vertices[index0].Position.xyz;
			const FPoint3& p1 = vertices[index1].Position.xyz;
			const FPoint3& p2 = vertices[index2].Position.xyz;
			const FVector2& uv0 = vertices[index0].Uv;
			const FVector2& uv1 = vertices[index1].Uv;
			const FVector2& uv2 = vertices[index2].Uv;

			const FVector3 edge0 = p1 - p0;
			const FVector3 edge1 = p2 - p0;
			const FVector2 diffX = FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
			const FVector2 diffY = FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
			const float r = 1.f / Cross(diffX, diffY);

			const FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
			vertices[index0].Tangent += tangent;
			vertices[index1].Tangent += tangent;
			vertices[index2].Tangent += tangent;
		}
		//Fix the tangents per vertex now because we accumulated
		for (auto& v : vertices)
		{
			v.Tangent = GetNormalized(Reject(v.Tangent, v.Normal));
		}

		return true;
	}
}
