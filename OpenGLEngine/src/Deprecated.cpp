//THIS FILE DOES NOTHING
//It simply holds deprecated code that is correct, but not currently in use

#if 0
//This can be used to load tangents ourself instead of using assimp

//TODO look at this
//this works for a triangle, but we are not using the face information here - need to take it into account
//We could do this for each face
//alternatively perhaps we could leave it as is, but average over the tangents and bitangents for each vertex
//in a face
//I believe looping over the faces might be the only appropriate way to do it in our case

//Loop over the faces of the object (so it needs indices)
//For each face, get bitangents and store them in respective indice (or += if array init to 0)
//Then chuck this into a buffer
void SceneInfo::ComputeTangentBasis(
  const std::vector<GLfloat> &vertices,
  const std::vector<GLfloat> &uvs,
  const std::vector<GLfloat> &normals,
  const std::vector<GLuint> &indices,
  glm::vec3* tangents,
  glm::vec3* bitangents
) {
  assert(indices.size() % 3 == 0);
  for (int i = 0; i < indices.size(); i += 3) {
    glm::vec3 v0 = core::make_vertex3(3 * indices[i], vertices);
    glm::vec3 v1 = core::make_vertex3(3 * indices[i + 1], vertices);
    glm::vec3 v2 = core::make_vertex3(3 * indices[i + 2], vertices);

    glm::vec2 uv0 = core::make_vertex2(2 * indices[i], uvs);
    glm::vec2 uv1 = core::make_vertex2(2 * indices[i + 1], uvs);
    glm::vec2 uv2 = core::make_vertex2(2 * indices[i + 2], uvs);

    //The edges of the triangles including v0
    glm::vec3 delta_pos1 = v1 - v0;
    glm::vec3 delta_pos2 = v2 - v0;

    //Get the corresponding uv edges
    glm::vec2 delta_uv1 = uv1 - uv0;
    glm::vec2 delta_uv2 = uv2 - uv0;

    //Compute the tangent and the bitangent
    float r = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
    glm::vec3 tangent = (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y)*r;
    //glm::vec3 bitangent = (delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x)*r;

    tangents[indices[i]] += tangent;
    tangents[indices[i + 1]] += tangent;
    tangents[indices[i + 2]] += tangent;

    /*    bitangent = glm::cross(tangent, normal);
    bitangents[indices[i]] += bitangent;
    bitangents[indices[i + 1]] += bitangent;
    bitangents[indices[i + 2]] += bitangent;*/
  }
  for (int i = 0; i < indices.size(); ++i) {
    glm::vec3 tangent = tangents[indices[i]];
    glm::vec3 normal = core::make_vertex3(3 * indices[i], normals);
    tangents[indices[i]] = glm::normalize(tangent - normal * glm::dot(normal, tangent));
    assert(glm::dot(tangents[indices[i]], normal) < 0.01f);
    bitangents[indices[i]] = glm::cross(normal, tangent);
  }
}

#endif