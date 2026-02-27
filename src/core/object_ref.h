//
// Created by Samarth Mahendra on 12/21/25.
//

#ifndef OBJECT_REF_H
#define OBJECT_REF_H


#include <string>

namespace orion {

    using ObjectId = std::string;

    struct ObjectRef {
        ObjectId id;
    };
}

#endif //OBJECT_REF_H
