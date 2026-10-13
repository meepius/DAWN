#include "juce/VisualizerDataQueue.h"

#include <algorithm>

namespace dawn {

VisualizerDataQueue::VisualizerDataQueue() : fifo_(8192) {}

void VisualizerDataQueue::push(const float* left, const float* right, int samples) noexcept {
    if (left == nullptr || samples <= 0) return;
    int start1 = 0;
    int size1 = 0;
    int start2 = 0;
    int size2 = 0;
    fifo_.prepareToWrite(samples, start1, size1, start2, size2);
    const int accepted = size1 + size2;
    if (accepted <= 0) return;
    for (int i = 0; i < size1; ++i) {
        left_[start1 + i] = left[i];
        right_[start1 + i] = right != nullptr ? right[i] : left[i];
    }
    for (int i = 0; i < size2; ++i) {
        left_[start2 + i] = left[size1 + i];
        right_[start2 + i] = right != nullptr ? right[size1 + i] : left[size1 + i];
    }
    fifo_.finishedWrite(accepted);
}

int VisualizerDataQueue::pop(float* left, float* right, int maxSamples) noexcept {
    if (left == nullptr || right == nullptr || maxSamples <= 0) return 0;
    const int ready = fifo_.getNumReady();
    if (ready <= 0) return 0;
    int start1 = 0;
    int size1 = 0;
    int start2 = 0;
    int size2 = 0;
    fifo_.prepareToRead(std::min(ready, maxSamples), start1, size1, start2, size2);
    for (int i = 0; i < size1; ++i) {
        left[i] = left_[start1 + i];
        right[i] = right_[start1 + i];
    }
    for (int i = 0; i < size2; ++i) {
        left[size1 + i] = left_[start2 + i];
        right[size1 + i] = right_[start2 + i];
    }
    fifo_.finishedRead(size1 + size2);
    return size1 + size2;
}

} // namespace dawn
