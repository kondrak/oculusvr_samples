struct LeapData;

class LeapMotion
{
public:
    LeapMotion() : m_leapData(nullptr), m_renderCameraImage(false)
    {
    }

    ~LeapMotion();
    void OnUpdate();
    void OnRender();

    void Init();
    void Destroy();
private:    
    void RecalculateSkeletonHands();
    void UpdateCameraImage();
    void RenderSkeletonHands();
    void ProcessGestures();

    void SetupCameraImageTexture();
    void RenderCameraImage();

    LeapData* m_leapData;
    bool m_renderCameraImage;
};