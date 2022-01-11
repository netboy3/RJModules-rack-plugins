#include "RJModules.hpp"

#include "common.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

/*
Thanks to Strum for the display widget!
*/

struct Displays: Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        CH2_INPUT,
        CH3_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float display_val[3];

    Displays() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);}
    void step() override;
};

struct DisplaysDisplayWidget : TransparentWidget {
  Displays *module;
  int display_num;


  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // text
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
    if (font) {
    nvgFontSize(vg, 32);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);
    }
    std::stringstream to_display;
    float display_val = 0.0;
    if (module)
      display_val = module->display_val[display_num];
    to_display = format4display(display_val);

    Vec textPos = Vec(16.0f, 33.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);

    nvgFontSize(vg, 16);
    textPos = Vec(1.0f, (display_val<0?20.0f:30.0f));
    nvgText(vg, textPos.x, textPos.y, (display_val<0?"-":"+"), NULL);
  }
};

void Displays::step() {

    display_val[0] = inputs[CH1_INPUT].value;
    outputs[CH1_OUTPUT].value = inputs[CH1_INPUT].value;

    display_val[1] = inputs[CH2_INPUT].value;
    outputs[CH2_OUTPUT].value = inputs[CH2_INPUT].value;

    display_val[2] = inputs[CH3_INPUT].value;
    outputs[CH3_OUTPUT].value = inputs[CH3_INPUT].value;

}

struct DisplaysWidget: ModuleWidget {
    DisplaysWidget(Displays *module);
};

DisplaysWidget::DisplaysWidget(Displays *module) {
		setModule(module);
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Displays.svg")));
        addChild(panel);
    }

    addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

    DisplaysDisplayWidget *display = new DisplaysDisplayWidget();
    display->box.pos = Vec(28, 60);
    display->box.size = Vec(100, 40);
    display->module = module;
    display->display_num = 0;
    addChild(display);

    addInput(createInput<PJ301MPort>(Vec(35, 123), module, Displays::CH1_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(95, 123), module, Displays::CH1_OUTPUT));

    DisplaysDisplayWidget *display2 = new DisplaysDisplayWidget();
    display2->box.pos = Vec(28, 160);
    display2->box.size = Vec(100, 40);
    display2->module = module;
    display2->display_num = 1;
    addChild(display2);

    addInput(createInput<PJ301MPort>(Vec(35, 223), module, Displays::CH2_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(95, 223), module, Displays::CH2_OUTPUT));

    DisplaysDisplayWidget *display3 = new DisplaysDisplayWidget();
    display3->box.pos = Vec(28, 260);
    display3->box.size = Vec(100, 40);
    display3->module = module;
    display3->display_num = 2;
    addChild(display3);

    addInput(createInput<PJ301MPort>(Vec(35, 323), module, Displays::CH3_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(95, 323), module, Displays::CH3_OUTPUT));

}

Model *modelDisplays = createModel<Displays, DisplaysWidget>("Displays");
