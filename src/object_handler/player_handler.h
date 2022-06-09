#include <pspctrl.h>

#include "object_handler.h" 
#include "projectile_handler.h"

class PlayerHandler : public ObjectHandler
{
private:
    SceCtrlData ctrlData;
    Image rocket = Image("assets/missile.bmp");

    ProjectileHandler * projectile_handler;
public:
    Person player = Person(Image("assets/player.bmp"));
    void init();
    void read_controls();
    void update_physics();
    void draw();
    PlayerHandler(int _velocity, ProjectileHandler * _projectile_handler);
    ~PlayerHandler();
};
