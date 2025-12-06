#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Pina {
    class Camera;
    class Input;
    class OrbitCamera;
    class FreelookCamera;
}

namespace PinaEditor {

/// Editor camera with orbit and fly modes
/// Delegates to core engine OrbitCamera and FreelookCamera controllers
class EditorCamera {
public:
    enum class Mode { Orbit, Fly };

    EditorCamera();
    ~EditorCamera();

    // Update camera based on input
    void update(Pina::Input* input, float deltaTime);

    // Focus on a point with given bounding size
    void focusOn(const glm::vec3& center, float size);

    // Reset to default position
    void reset();

    // Get the underlying camera
    Pina::Camera* getCamera() { return m_camera.get(); }

    // Camera mode
    Mode getMode() const { return m_mode; }
    void setMode(Mode mode);

    // Orbit settings (delegates to OrbitCamera)
    void setOrbitTarget(const glm::vec3& target);
    glm::vec3 getOrbitTarget() const;

    // Speed settings
    void setMoveSpeed(float speed);
    void setRotateSpeed(float speed);
    void setZoomSpeed(float speed);

    // Get underlying controllers for advanced configuration
    Pina::OrbitCamera* getOrbitController() { return m_orbitController.get(); }
    Pina::FreelookCamera* getFreelookController() { return m_freelookController.get(); }

private:
    void transferStateToOrbit();
    void transferStateToFreelook();

    std::unique_ptr<Pina::Camera> m_camera;
    std::unique_ptr<Pina::OrbitCamera> m_orbitController;
    std::unique_ptr<Pina::FreelookCamera> m_freelookController;

    Mode m_mode = Mode::Orbit;
};

} // namespace PinaEditor
