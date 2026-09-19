#ifndef TIMELINE_H
#define TIMELINE_H

struct Keyframe {
	int frame;
	// Add other properties for keyframe, such as position, rotation, scale, etc.
	int modelIndex; // Index of the model this keyframe belongs to
};

class Timeline {
private:
	int startFrame;
	int endFrame;
	bool isPlaying;
public:
	Timeline(int start, int end) : startFrame(start), endFrame(end), isPlaying(false) {}
	void Play() { isPlaying = true; }
	void Pause() { isPlaying = false; }
	void Stop() { isPlaying = false; /* Reset to start frame if needed */ }
	void AddKeyframe(const Keyframe& keyframe) {
		// Add keyframe to the timeline
	}
	// Other methods to manage keyframes and playback
};

#endif // !TIMELINE_H