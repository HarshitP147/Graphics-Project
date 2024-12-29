#define BUFFER_OFFSET(i) ((char *)NULL + (i))


class Robot {
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;

    std::vector<GLuint> textureIDs;

	tinygltf::Model model;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vertexArrayObject;
		std::map<int, GLuint> vertexBufferObjects;
	};
	std::vector<PrimitiveObject> primitiveObjects;

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
            std::vector<glm::mat4> &localTransforms)
        {
            // ---------------------------------------
            // TODO: your code here
            // ---------------------------------------
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
            std::vector<glm::mat4> &globalTransforms)
        {
            // ----------------------------------------
            // TODO: your code here
            // ----------------------------------------

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

            // In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

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

                skinObject.globalJointTransforms.resize(skin.joints.size());
                skinObject.jointMatrices.resize(skin.joints.size());

                // ----------------------------------------------
                // TODO: your code here to compute joint matrices
                // ----------------------------------------------

                std::vector<glm::mat4> localNodeTransforms(skin.joints.size());

                glm::mat4 parentTransform(1.0f);
                std::vector<glm::mat4> globalNodeTransforms(skin.joints.size());

                int rootNodeIndex = skin.joints[0];

                // Compute local transforms at each node
                computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

                // Compute global transforms at each node
                computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex, parentTransform, globalNodeTransforms);
                skinObject.globalJointTransforms = globalNodeTransforms;

                for (size_t j = 0; j < skinObject.globalJointTransforms.size(); ++j) {
                    int jointNodeIndex = skin.joints[j];
                    skinObject.jointMatrices[j] = skinObject.globalJointTransforms[jointNodeIndex] * skinObject.inverseBindMatrices[j];
                }

                // ----------------------------------------------

                skinObjects.push_back(skinObject);
            }
            return skinObjects;
        }

        int findKeyframeIndex(const std::vector<float>& times, float animationTime)
        {
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

        std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model)
        {
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

        void updateAnimation(
            const tinygltf::Model &model,
            const tinygltf::Animation &anim,
            const AnimationObject &animationObject,
            float time,
            std::vector<glm::mat4> &nodeTransforms)
        {
            // There are many channels so we have to accumulate the transforms
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

                // ----------------------------------------------------------
                // TODO: Find a keyframe for getting animation data
                // ----------------------------------------------------------
                int keyframeIndex = this->findKeyframeIndex(times, animationTime);

                const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
                const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

                // -----------------------------------------------------------
                // TODO: Add interpolation for smooth animation
                // -----------------------------------------------------------

                float t0 = times[keyframeIndex];
                float t1 = times[keyframeIndex+1];

                float factor = (animationTime-t1)/(t0-t1);


                if (channel.target_path == "translation") {
                    glm::vec3 translation0, translation1;
                    memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
                    memcpy(&translation1, outputPtr + (keyframeIndex+1) * 3 * sizeof(float), 3 * sizeof(float));

                    // glm::vec3 translation = glm::mix(translation0,translation1,factor);
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

                    // glm::quat rotation = rotation0;
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

                    // glm::vec3 scale = glm::mix(scale0,scale1,factor);
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

            // -------------------------------------------------
            // TODO: Recompute joint matrices
            // -------------------------------------------------
            for(size_t i=0; i<skinObjects.size(); i++){
                SkinObject &skinObject = skinObjects[i];
                for(size_t j=0; j<skinObject.jointMatrices.size(); j++){
                    int jointIndex = model.skins[i].joints[j];
                    glm::mat4 globalTransform = nodeTransforms[jointIndex];
                    skinObject.jointMatrices[j] = globalTransform * skinObject.inverseBindMatrices[j];
                }
            }

            // Upload to shader
            for(size_t i=0; i<skinObjects.size(); i++){
                if(!skinObjects[i].jointMatrices.empty()){
                    glUniformMatrix4fv(jointMatricesID, skinObjects[i].jointMatrices.size(), GL_FALSE, glm::value_ptr(skinObjects[i].jointMatrices[0]));
                }
            }
        }

        void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

            for (size_t i = 0; i < mesh.primitives.size(); ++i)
            {
                GLuint vertexArrayObject = primitiveObjects[i].vertexArrayObject;
                std::map<int, GLuint> vbos = primitiveObjects[i].vertexBufferObjects;

                glBindVertexArray(vertexArrayObject);

                tinygltf::Primitive primitive = mesh.primitives[i];
                tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

                glDrawElements(primitive.mode, indexAccessor.count,
                            indexAccessor.componentType,
                            BUFFER_OFFSET(indexAccessor.byteOffset));

                glBindVertexArray(0);
            }
        }

        void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
                            tinygltf::Model &model, tinygltf::Node &node) {
            // Draw the mesh at the node, and recursively do so for children nodes
            if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
                drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
            }
            for (size_t i = 0; i < node.children.size(); i++) {
                drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
            }
        }
        void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
                    tinygltf::Model &model) {
            // Draw all nodes
            const tinygltf::Scene &scene = model.scenes[model.defaultScene];
            for (size_t i = 0; i < scene.nodes.size(); ++i) {
                drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
            }
        }

        void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {

            std::map<int, GLuint> vbos;
            for (size_t i = 0; i < model.bufferViews.size(); ++i) {
                const tinygltf::BufferView &bufferView = model.bufferViews[i];

                int target = bufferView.target;

                if (bufferView.target == 0) {
                    continue;
                }

                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
                GLuint vbo;
                glGenBuffers(1, &vbo);
                glBindBuffer(target, vbo);
                glBufferData(target, bufferView.byteLength,
                            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

                vbos[i] = vbo;
            }

            // Each mesh can contain several primitives (or parts), each we need to
            // bind to an OpenGL vertex array object
            for (size_t i = 0; i < mesh.primitives.size(); ++i) {

                tinygltf::Primitive primitive = mesh.primitives[i];
                tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

                GLuint vertexArrayObject;
                glGenVertexArrays(1, &vertexArrayObject);
                glBindVertexArray(vertexArrayObject);

                for (auto &attrib : primitive.attributes) {
                    tinygltf::Accessor accessor = model.accessors[attrib.second];
                    int byteStride =
                        accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                    glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

                    int size = 1;
                    if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                        size = accessor.type;
                    }

                    int vaa = -1;
                    if (attrib.first.compare("POSITION") == 0) vaa = 0;
                    if (attrib.first.compare("NORMAL") == 0) vaa = 1;
                    if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
                    if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
                    if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
                    if (vaa > -1) {
                        glEnableVertexAttribArray(vaa);
                        glVertexAttribPointer(vaa, size, accessor.componentType,
                                            accessor.normalized ? GL_TRUE : GL_FALSE,
                                            byteStride, BUFFER_OFFSET(accessor.byteOffset));
                    } else {
                        std::cout << "vaa missing: " << attrib.first << std::endl;
                    }
                }

                // Record VAO for later use
                PrimitiveObject primitiveObject;
                primitiveObject.vertexArrayObject = vertexArrayObject;
                primitiveObject.vertexBufferObjects = vbos;
                primitiveObjects.push_back(primitiveObject);

                glBindVertexArray(0);
            }
        }

        void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
                            tinygltf::Model &model,
                            tinygltf::Node &node) {
            // Bind buffers for the current mesh at the node
            if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
                bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
            }

            // Recursive into children nodes
            for (size_t i = 0; i < node.children.size(); i++) {
                assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
                bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
            }
        }

        std::vector<PrimitiveObject> bindModel(tinygltf::Model &model) {
            std::vector<PrimitiveObject> primitiveObjects;

            const tinygltf::Scene &scene = model.scenes[model.defaultScene];
            for (size_t i = 0; i < scene.nodes.size(); ++i) {
                assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
                bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
            }

            return primitiveObjects;
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

            CheckOpenGLErrors("Loading models");

            return res;
        }

        std::vector<GLuint> loadGLTFTextures(const tinygltf::Model &model){
            std::vector<GLuint> textureIDs;

            for(auto &tex: model.textures){
                if (tex.source < 0 || tex.source >= model.images.size()) {
                    continue;
                }

                const auto &img = model.images[tex.source];

                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);

                GLenum format = GL_RGB;
                if(img.component == 4) format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_2D, 0, format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.image.data());
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                textureIDs.push_back(textureID);
            }

            return textureIDs;
        }

        void initialize() {
            // Modify your path if needed
            // if(!loadModel(model, "../src/assets/models/bot.gltf")){
            //     return;
            // }
            if(!loadModel(model, "../src/assets/models/stormtrooper/scene.gltf")){
                return;
            }


            CheckOpenGLErrors("Loading model");

            // Prepare buffers for rendering
            primitiveObjects = bindModel(model);

            // Prepare joint matrices
            skinObjects = prepareSkinning(model);

            // Prepare animation data
            animationObjects = prepareAnimation(model);

            // Load textures
            textureIDs = loadGLTFTextures(model);


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

        Robot(){
            initialize();
        }

        void update(float time) {
            // return;
            // -------------------------------------------------
            // TODO: your code here
            // -------------------------------------------------
            if(model.animations.size() > 0){
                const tinygltf::Animation &animation = model.animations[0];
                const AnimationObject &animationObject = animationObjects[0];

                const tinygltf::Skin &skin = model.skins[0];
                std::vector<glm::mat4> nodeTransforms(skin.joints.size());
                for(size_t i=0;i<nodeTransforms.size();i++){
                    nodeTransforms[i] = glm::mat4(1.0);
                }

                updateAnimation(model, animation, animationObject, time, nodeTransforms);

                // Recomputing joint global and local transforms
                std::vector<glm::mat4> globalNodeTransforms(skin.joints.size(), glm::mat4(1.0f));

                int rootNode = skin.joints[0];

                // computeGlobalNodeTransform(model, nodeTransforms, rootNode, glm::mat4(1.0f), &skinObjects[0].globalJointTransforms);

            }
        }

	    void render(glm::mat4 cameraMatrix) {
            glUseProgram(programID);

            // Set camera
            glm::mat4 mvp = cameraMatrix;
            glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

            CheckOpenGLErrors("Setting camera");

            // -----------------------------------------------------------------
            // TODO: Set animation data for linear blend skinning in shader
            // -----------------------------------------------------------------

            // First we set the joint matrices
            for(size_t i=0; i<skinObjects.size(); i++){
                // if(!skinObjects[i].jointMatrices.empty()){
                // }
                glUniformMatrix4fv(jointMatricesID, skinObjects[i].jointMatrices.size(), GL_FALSE, glm::value_ptr(skinObjects[i].jointMatrices[0]));
            }

            CheckOpenGLErrors("Animation data setup");

            // Bind the first texture sampler
            if(!textureIDs.empty()){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
                glUniform1i(glGetUniformLocation(programID, "textureSampler"), 0);
            }

            // -----------------------------------------------------------------

            // Set light data
            glUniform3fv(lightPositionID, 1, &glm::vec3(-275.0f, 500.0f, 800.0)[0]);
            glUniform3fv(lightIntensityID, 1, &glm::vec3(5e6, 5e6, 5e6)[0]);

            CheckOpenGLErrors("Setting light");




            // Draw the GLTF model
            drawModel(primitiveObjects, model);
	    }

        void cleanup() {
            glDeleteProgram(programID);
        }
};