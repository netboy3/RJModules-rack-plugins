/*
Succccccccccccccc

*/

#include "RJModules.hpp"

using namespace rack;

#define NUM_OF_IMAGES 2

struct Succulent : Module {
    int Style = 0;

    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "style", json_integer(Style));
        return rootJ;
    }
    void dataFromJson(json_t *rootJ) override {
        json_t *StyleJ = json_object_get(rootJ, "style");
        if (StyleJ)
            Style = json_integer_value(StyleJ);
        if (Style >= NUM_OF_IMAGES || Style < 0)
            Style = 0;
    }
};

struct SucculentPanel : TransparentWidget {
    std::string *imageListPtr;
    int *stylePtr;

    void draw(const DrawArgs &args) override {
        std::shared_ptr<Image> image = APP->window->loadImage(*(imageListPtr + *stylePtr));
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
        if (image) {
            NVGpaint paint = nvgImagePattern(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0, image->handle, 1.0);
            nvgFillPaint(args.vg, paint);
            nvgFill(args.vg);
        }
        Widget::draw(args);
    }
};

struct SucculentWidget : ModuleWidget {
    SucculentPanel *imagePanel;
    std::string imageList[NUM_OF_IMAGES];
    std::string *imageListPtr = imageList;
    int defaultStyle = 0;

    void appendContextMenu(Menu *menu) override {
        Succulent *succc = dynamic_cast<Succulent *>(module);
        assert(succc);
        menu->addChild(construct<MenuEntry>());
        menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Style"));
        menu->addChild(createCheckMenuItem(
            "Succccc", "",
            [=]() { return succc->Style == 0; },
            [=]() { succc->Style = 0; }));
        menu->addChild(createCheckMenuItem(
            "Meow", "",
            [=]() { return succc->Style == 1; },
            [=]() { succc->Style = 1; }));
    }

    SucculentWidget(Succulent *module);
};

SucculentWidget::SucculentWidget(Succulent *module) {
    setModule(module);
    imageList[0] = asset::plugin(pluginInstance, "res/Blank_20HP.png");
    imageList[1] = asset::plugin(pluginInstance, "res/Zen_20HP.png");
    box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    imagePanel = new SucculentPanel();
    imagePanel->box.size = box.size;
    imagePanel->imageListPtr = imageListPtr;
    if (module) {
        imagePanel->stylePtr = &module->Style;
    } else {
        imagePanel->stylePtr = &defaultStyle;
    }
    addChild(imagePanel);
}

Model *modelSucculent = createModel<Succulent, SucculentWidget>("Succulent");