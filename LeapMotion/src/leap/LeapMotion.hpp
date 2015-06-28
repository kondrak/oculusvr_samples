struct LeapData;

class LeapMotion
{
public:
    LeapMotion() : m_leapData(nullptr)
    {
    }

    ~LeapMotion();
    
    void OnRender();

    void Init();
    void Destroy();
private:    
    void RecalculateSkeletonHands();
    void RenderSkeletonHands();

    void SetupCameraImageTexture();
    void RenderCameraImage();

    LeapData* m_leapData;
};