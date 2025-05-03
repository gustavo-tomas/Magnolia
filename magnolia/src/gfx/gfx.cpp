#include "gfx/gfx.hpp"

namespace mag
{
    namespace gfx
    {
        struct GfxState
        {
                unique<IDevice> device;
        };

        static GfxState* state = nullptr;

        b8 initialize()
        {
            state = new GfxState();
            state->device = create_device();

            return state->device != nullptr;
        }

        void shutdown() { delete state; }

        void on_update(const f32 dt)
        {
            (void)dt;
            state->device->draw_frame();
        }
    };  // namespace gfx
};      // namespace mag
