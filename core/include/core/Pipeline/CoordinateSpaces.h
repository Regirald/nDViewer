#pragma once

namespace ndv::pipeline
{
    // Coordinate space stages used in the rendering pipeline.
    enum class CoordinateSpace
    {
        Object,
        World,
        View,
        Clip,
        Ndc,
        Screen
    };

    // Object: local mesh/model coordinates.
    // World: global scene coordinates after Model matrix.
    // View: camera-relative coordinates after View matrix (inverse of camera transform).
    // Clip: homogeneous clip coordinates after Projection matrix.
    // NDC: normalized device coordinates after perspective divide.
    // Screen: final pixel coordinates after viewport mapping.
}
