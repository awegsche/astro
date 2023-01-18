#ifndef DARK_H_
#define DARK_H_

#include "image.h"

class Dark : public ImageFile {
    public:
    static std::optional<Dark> load(ImagePath const &path, std::unique_ptr<LibRaw> const &processor) {
        Dark l{};
        if (!l.image_from_image_path(path, processor))
            return {};
        return l;
    }
};

#endif // DARK_H_
