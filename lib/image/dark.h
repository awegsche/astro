#ifndef DARK_H_
#define DARK_H_

#include "image.h"

template<typename TImage> 
class Dark {
    public:
        Dark(fs::path const& path) : m_image(path) {}

    private:
        TImage m_image;
};
#endif // DARK_H_
