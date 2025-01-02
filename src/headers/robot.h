#define BUFFER_OFFSET(i) ((char *)NULL + (i))


class Robot {
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;

    glm::vec3 position;
    GLfloat angle;

    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

	tinygltf::Model model;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vertexArrayObject;
		std::map<int, GLuint> vertexBufferObjects;
	};
	std::vector<PrimitiveObject> primitiveObjects;

    struct MeshData{
        std::vector<int> primitiveIndices;
    };
    std::vector<MeshData> allMeshData;

	// Skinning
	struct SkinObject {
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<glm::mat4> globalJointTransforms;
		std::vector<glm::mat4> jointMatrices;
	};
	std::vector<SkinObject> skinObjects;

	// Animation
	struct AnimationSamplerObject {
		std::vector<float> input;
		std::vector<glm::vec4> output;
		int interpolation;
	};
	struct AnimationChannelObject {
		int sampler;
		std::string targetPath;
		int targetNode;
	};
	struct AnimationObject {
		std::vector<AnimationSamplerObject> samplers;	// Animation data
		std::vector<AnimationChannelObject> channels;	// Animation channels
	};
	std::vector<AnimationObject> animationObjects;

    private:
        glm::mat4 getNodeTransform(const tinygltf::Node& node) {
            glm::mat4 transform(1.0f);

            if (node.matrix.size() == 16) {
                transform = glm::make_mat4(node.matrix.data());
            } else {
                if (node.translation.size() == 3) {
                    transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
                }
                if (node.rotation.size() == 4) {
                    glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                    transform *= glm::mat4_cast(q);
                }
                if (node.scale.size() == 3) {
                    transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
                }
            }
            return transform;
        }

        void computeLocalNodeTransform(const tinygltf::Model& model,
            int nodeIndex,
            std::vector<glm::mat4> &localTransforms){
            const tinygltf::Node &node = model.nodes[nodeIndex];

            localTransforms[nodeIndex] = getNodeTransform(node);

            if(!node.children.empty()){
                for(int childNode: node.children){
                    computeLocalNodeTransform(model, childNode, localTransforms);
                }
            }
        }

        void computeGlobalNodeTransform(const tinygltf::Model& model,
            const std::vector<glm::mat4> &localTransforms,
            int nodeIndex, const glm::mat4& parentTransform,
            std::vector<glm::mat4> &globalTransforms){
            // Find the global transformations
            globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];

            // Now going to the children node
            const tinygltf::Node &node = model.nodes[nodeIndex];
            if(!node.children.empty()){
                for(int childNode: node.children){
                    computeGlobalNodeTransform(model, localTransforms, childNode, globalTransforms[nodeIndex], globalTransforms);
                }
            }
        }

        std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model) {
            std::vector<SkinObject> skinObjects;


            for (size_t i = 0; i < model.skins.size(); i++) {
                SkinObject skinObject;

                const tinygltf::Skin &skin = model.skins[i];

                // Read inverseBindMatrices
                const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
                assert(accessor.type == TINYGLTF_TYPE_MAT4);
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
                const float *ptr = reinterpret_cast<const float *>(
                    buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);

                skinObject.inverseBindMatrices.resize(accessor.count);
                for (size_t j = 0; j < accessor.count; j++) {
                    float m[16];
                    memcpy(m, ptr + j * 16, 16 * sizeof(float));
                    skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
                }

                assert(skin.joints.size() == accessor.count);

                skinObject.globalJointTransforms.resize(model.nodes.size());
                skinObject.jointMatrices.resize(model.nodes.size());

                std::vector<glm::mat4> localTransforms(model.nodes.size());
                std::vector<glm::mat4> globalTransforms(model.nodes.size());

                int rootNodeIndex = skin.joints[0];

                // Compute local transforms at each node
                computeLocalNodeTransform(model, rootNodeIndex, localTransforms);

                // Compute global transforms at each node
                computeGlobalNodeTransform(model, localTransforms, rootNodeIndex, glm::mat4(1.0f), globalTransforms);
                skinObject.globalJointTransforms = globalTransforms;

                for(size_t j=0; j<skin.joints.size(); j++){
                    int jointNodeIndex = skin.joints[j];
                    skinObject.jointMatrices[j] = skinObject.globalJointTransforms[jointNodeIndex] * skinObject.inverseBindMatrices[j];
                }

                skinObjects.push_back(skinObject);
            }
            return skinObjects;
        }

        // Called once after model is loaded
        void bindModel(tinygltf::Model &model){
            allMeshData.resize(model.meshes.size());

            const tinygltf::Scene &scene = model.scenes[model.defaultScene];
            for (int rootNodeIndex : scene.nodes) {
                bindModelNodes(model, rootNodeIndex);
            }
        }
        // Recursively process a node and its children
        void bindModelNodes(tinygltf::Model &model, int nodeIndex){
            const tinygltf::Node &node = model.nodes[nodeIndex];

            // If this node references a mesh, bind it
            if (node.mesh >= 0 && node.mesh < (int)model.meshes.size()) {
                bindMesh(model, node.mesh);
            }

            // Then recurse for children
            for (int childIndex : node.children) {
                bindModelNodes(model, childIndex);
            }
        }

        void bindMesh(tinygltf::Model &model, int meshIndex){
            // Retrieve the glTF mesh
            tinygltf::Mesh &mesh = model.meshes[meshIndex];
            // Reference to the MeshData entry for this mesh
            MeshData &meshData = allMeshData[meshIndex];

            size_t startIndex = primitiveObjects.size();

            for (size_t p = 0; p < mesh.primitives.size(); p++) {
                const tinygltf::Primitive &primitive = mesh.primitives[p];

                GLuint vao;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                std::map<int, GLuint> vbos;

                for (auto &attrib : primitive.attributes) {
                    int accessorIndex = attrib.second;
                    const tinygltf::Accessor &accessor = model.accessors[accessorIndex];
                    const tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buf = model.buffers[bv.buffer];

                    if (vbos.find(accessor.bufferView) == vbos.end()) {
                        GLuint vbo;
                        glGenBuffers(1, &vbo);
                        glBindBuffer(bv.target, vbo);
                        glBufferData(bv.target, bv.byteLength,
                                    &buf.data[bv.byteOffset], GL_STATIC_DRAW);
                        vbos[accessor.bufferView] = vbo;
                    } else {
                        glBindBuffer(bv.target, vbos[accessor.bufferView]);
                    }

                    // Now set up glVertexAttribPointer
                    int byteStride = accessor.ByteStride(bv);
                    int size = 1;
                    if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                        size = accessor.type;
                    }
                    int location = -1;
                    if (attrib.first == "POSITION")  location = 0;
                    if (attrib.first == "NORMAL")    location = 1;
                    if (attrib.first == "TEXCOORD_0")location = 2;
                    if (attrib.first == "JOINTS_0")  location = 3;
                    if (attrib.first == "WEIGHTS_0") location = 4;
                    if (attrib.first == "COLOR_0")   location = 5;

                    if (location >= 0) {
                        glEnableVertexAttribArray(location);
                        glVertexAttribPointer(location,
                                            size,
                                            accessor.componentType,
                                            accessor.normalized ? GL_TRUE : GL_FALSE,
                                            byteStride,
                                            (void*)(uintptr_t)accessor.byteOffset);
                    }
                }

                {
                    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView &ibv = model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer &ibuf = model.buffers[ibv.buffer];

                    if (vbos.find(indexAccessor.bufferView) == vbos.end()) {
                        GLuint ibo;
                        glGenBuffers(1, &ibo);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                    ibv.byteLength,
                                    &ibuf.data[ibv.byteOffset],
                                    GL_STATIC_DRAW);
                        vbos[indexAccessor.bufferView] = ibo;
                    } else {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[indexAccessor.bufferView]);
                    }
                }

                glBindVertexArray(0);

                PrimitiveObject primObj;
                primObj.vertexArrayObject = vao;
                primObj.vertexBufferObjects = vbos;

                primitiveObjects.push_back(primObj);
            }

            // Now record which globalPrimitives indices belong to this mesh:
            for (size_t p = 0; p < mesh.primitives.size(); p++) {
                // The newly pushed-back primitives start at startIndex
                meshData.primitiveIndices.push_back((int)(startIndex + p));
            }
        }

        std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model){
            std::vector<AnimationObject> animationObjects;
            for (const auto &anim : model.animations) {
                AnimationObject animationObject;

                for (const auto &sampler : anim.samplers) {
                    AnimationSamplerObject samplerObject;

                    const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
                    const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
                    const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

                    assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                    assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

                    // Input (time) values
                    samplerObject.input.resize(inputAccessor.count);

                    const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
                    const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

                    // Read input (time) values
                    int stride = inputAccessor.ByteStride(inputBufferView);
                    for (size_t i = 0; i < inputAccessor.count; ++i) {
                        samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
                    }

                    const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
                    const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
                    const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

                    assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                    const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
                    const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

                    int outputStride = outputAccessor.ByteStride(outputBufferView);

                    // Output values
                    samplerObject.output.resize(outputAccessor.count);

                    for (size_t i = 0; i < outputAccessor.count; ++i) {

                        if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                            memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
                        } else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
                            memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
                        } else {
                            std::cout << "Unsupport accessor type ..." << std::endl;
                        }

                    }

                    animationObject.samplers.push_back(samplerObject);
                }

                animationObjects.push_back(animationObject);
            }
            return animationObjects;
        }

        void drawMesh(const tinygltf::Model &model, int meshIndex){
            const tinygltf::Mesh &mesh = model.meshes[meshIndex];

            const std::vector<int> &primIndices = allMeshData[meshIndex].primitiveIndices;

            for (size_t p = 0; p < mesh.primitives.size(); p++) {
                int primID = primIndices[p]; // index into globalPrimitives
                const PrimitiveObject &primObj = primitiveObjects[primID];

                glBindVertexArray(primObj.vertexArrayObject);

                const tinygltf::Primitive &primitive = mesh.primitives[p];
                const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];

                GLuint ibo = primObj.vertexBufferObjects.at(indexAccessor.bufferView);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

                // Draw
                glDrawElements(primitive.mode,
                            indexAccessor.count,
                            indexAccessor.componentType,
                            (void*)(uintptr_t)indexAccessor.byteOffset);

                glBindVertexArray(0);
             }
        }

        void drawModelNodes(const tinygltf::Model &model, int nodeIndex){
            const tinygltf::Node &node = model.nodes[nodeIndex];

            // If this node references a mesh, draw it
            if (node.mesh >= 0 && node.mesh < (int)model.meshes.size()) {
                drawMesh(model, node.mesh);
            }

            // Recurse on children
            for (int childIndex : node.children) {
                drawModelNodes(model, childIndex);
            }
        }

        void drawModel(const tinygltf::Model &model){
            const tinygltf::Scene &scene = model.scenes[model.defaultScene];

            // For each root node in the scene
            for (int rootNodeIndex : scene.nodes) {
                drawModelNodes(model, rootNodeIndex);
            }
        }

        int findKeyframeIndex(const std::vector<float>& times, float animationTime){
            int left = 0;
            int right = times.size() - 1;

            while (left <= right) {
                int mid = (left + right) / 2;

                if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
                    return mid;
                }
                else if (times[mid] > animationTime) {
                    right = mid - 1;
                }
                else { // animationTime >= times[mid + 1]
                    left = mid + 1;
                }
            }

            // Target not found
            return times.size() - 2;
        }

        void updateAnimation(
            const tinygltf::Model &model,
            const tinygltf::Animation &anim,
            const AnimationObject &animationObject,
            float time,
            std::vector<glm::mat4> &nodeTransforms)
        {
            for (const auto &channel : anim.channels) {

                int targetNodeIndex = channel.target_node;
                const auto &sampler = anim.samplers[channel.sampler];

                // Access output (value) data for the channel
                const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
                const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
                const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

                // Calculate current animation time (wrap if necessary)
                const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
                float animationTime = fmod(time, times.back());

                int keyframeIndex = this->findKeyframeIndex(times, animationTime);

                const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
                const float *outputBuf = reinterpret_cast<const float*>(outputPtr);


                float t0 = times[keyframeIndex];
                float t1 = times[keyframeIndex+1];

                float factor = (animationTime  - t1)/ (t1 - t0);

                if (channel.target_path == "translation") {
                    glm::vec3 translation0, translation1;
                    memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
                    memcpy(&translation1, outputPtr + (keyframeIndex+1) * 3 * sizeof(float), 3 * sizeof(float));

                    glm::vec3 translation;
                    if(sampler.interpolation=="STEP"){
                        translation = translation1;
                    } else if(sampler.interpolation=="LINEAR"){
                        translation = glm::mix(translation0,translation1,factor);
                    }
                    nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
                } else if (channel.target_path == "rotation") {
                    glm::quat rotation0, rotation1;
                    memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
                    memcpy(&rotation1, outputPtr + (keyframeIndex+1) * 4 * sizeof(float), 4 * sizeof(float));

                    glm::quat rotation;
                    if(sampler.interpolation=="STEP"){
                        rotation = rotation1;
                    } else if(sampler.interpolation=="LINEAR"){
                        rotation = glm::slerp(rotation0,rotation1,factor);
                    }
                    nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
                } else if (channel.target_path == "scale") {
                    glm::vec3 scale0, scale1;
                    memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
                    memcpy(&scale1, outputPtr + (keyframeIndex+1) * 3 * sizeof(float), 3 * sizeof(float));

                    glm::vec3 scale;
                    if(sampler.interpolation=="STEP"){
                        scale = scale1;
                    } else if(sampler.interpolation=="LINEAR"){
                        scale = glm::mix(scale0,scale1,factor);
                    }
                    nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
                }
            }
        }

        void updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {
            for(size_t i=0; i<skinObjects.size(); i++){
                const tinygltf::Skin &skin = model.skins[i];

                std::vector<glm::mat4> globalNodeTransforms(model.nodes.size());

                int rootNodeIndex = skin.joints[0];

                computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex, glm::mat4(1.0f), globalNodeTransforms);
                skinObjects[i].globalJointTransforms = globalNodeTransforms;

                for(size_t j=0; j<skin.joints.size(); j++){
                    int jointIndex = skin.joints[j];
                    skinObjects[i].jointMatrices[j] = skinObjects[i].globalJointTransforms[jointIndex] * skinObjects[i].inverseBindMatrices[j];
                }
            }

        }

        bool loadModel(tinygltf::Model &model, const char *filename) {
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;

            bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

            if (!warn.empty()) {
                std::cout << "WARN: " << warn << std::endl;
            }

            if (!err.empty()) {
                std::cout << "ERR: " << err << std::endl;
            }

            if (!res)
                std::cout << "Failed to load glTF: " << filename << std::endl;
            else
                std::cout << "Loaded glTF: " << filename << std::endl;


            return res;
        }

        void initialize() {
            if(!loadModel(model, "../src/assets/models/bot/waving.gltf")){
                return;
            }

            // Prepare buffers for rendering
            bindModel(model);

            // Prepare joint matrices
            skinObjects = prepareSkinning(model);

            // Prepare animation data
            animationObjects = prepareAnimation(model);


            CheckOpenGLErrors("Loading model buffers");

            // Create and compile our GLSL program from the shaders
            programID = LoadShadersFromFile("../src/shaders/robot.vert", "../src/shaders/robot.frag");
            if (programID == 0)
            {
                std::cerr << "Failed to load shaders." << std::endl;
            }

            CheckOpenGLErrors("Loading shaders");

            // Get a handle for GLSL variables
            mvpMatrixID = glGetUniformLocation(programID, "MVP");
            lightPositionID = glGetUniformLocation(programID, "lightPosition");
            lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
            jointMatricesID = glGetUniformLocation(programID, "u_jointMatrix");

            CheckOpenGLErrors("Getting shader variables");
        }

    public:
        Robot(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), GLfloat angle = 0.0f, glm::vec3 lightPosition = glm::vec3(-275.0f, 500.0f, 800.0f), glm::vec3 lightIntensity = glm::vec3(5e6, 5e6, 5e6)) {
            this->position = position;
            this->angle = angle;
            this->lightPosition = lightPosition;
            this->lightIntensity = lightIntensity;
            initialize();
        }

        void update(float time) {
            if(model.animations.empty()){
                return;
            }

            const tinygltf::Animation &anim = model.animations[0];
            const AnimationObject &animationObject = animationObjects[0];


            const tinygltf::Skin &skin = model.skins[0];
            std::vector<glm::mat4> nodeTransforms(skin.joints.size());
            for(size_t i=0; i<nodeTransforms.size(); i++){
                nodeTransforms[i] = glm::mat4(1.0);
            }

            updateAnimation(model, anim, animationObject, time, nodeTransforms);

            updateSkinning(nodeTransforms);
        }

	    void render(glm::mat4 cameraMatrix) {
            glUseProgram(programID);

            // Set model matrix
            glm::mat4 modelMatrix = glm::mat4(1.0f);

            // Translate the model
            modelMatrix = glm::translate(modelMatrix, position);

            // Scale the model
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.025f));

            // Translate by Y axis
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 10.0f, 0.0f));

            // Rotate by X axis
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            // Rotation by Y axis
            modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

            // Set camera
            glm::mat4 mvp = cameraMatrix * modelMatrix;
            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            CheckOpenGLErrors("Setting camera");

            // First we set the joint matrices
            for(size_t i=0; i<skinObjects.size(); i++){
                glUniformMatrix4fv(jointMatricesID, skinObjects[i].jointMatrices.size(), GL_FALSE, glm::value_ptr(skinObjects[i].jointMatrices[0]));
            }

            CheckOpenGLErrors("Animation data setup");

            // -----------------------------------------------------------------

            // Set light data
            glUniform3fv(lightPositionID, 1, glm::value_ptr(lightPosition));
            glUniform3fv(lightIntensityID, 1, glm::value_ptr(lightIntensity));

            // Draw the GLTF model
            drawModel(model);
        }

        void cleanup() {
            glDeleteProgram(programID);
        }
};