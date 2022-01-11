#include "RJModules.hpp"

#include "dsp/digital.hpp"
#include "common.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

/*
Thanks to Strum for the display widget!
*/

struct Range: Module {
    enum ParamIds {
        CH1_PARAM,
        CH2_PARAM,
        CH3_PARAM,
        CH4_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float display_val[6];

    Range() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Range::CH1_PARAM, -12.0, 12.0, -12.0, "");
        configParam(Range::CH2_PARAM, -12.0, 12.0, 12.0, "");
        configParam(Range::CH3_PARAM, -12.0, 12.0, -12.0, "");
        configParam(Range::CH4_PARAM, -12.0, 12.0, 12.0, "");
    }
    void step() override;
};

void Range::step() {

    display_val[0] = params[CH1_PARAM].value;
    display_val[1] = params[CH2_PARAM].value;
    display_val[2] = params[CH3_PARAM].value;
    display_val[3] = params[CH4_PARAM].value;
    display_val[4] = inputs[CH1_PARAM].value;

    // new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
    float output = ( (inputs[CH1_PARAM].value - params[CH1_PARAM].value) / (params[CH2_PARAM].value - params[CH1_PARAM].value) ) * (params[CH4_PARAM].value - params[CH3_PARAM].value) + params[CH3_PARAM].value;

    display_val[5] = output;
    outputs[CH1_OUTPUT].value = output;

}

struct RangeDisplayWidget : TransparentWidget {

    Range *module;
    int display_num;

    void draw(NVGcontext *vg) override {
        // Background
        NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);

        // text
        std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
        if (font) {
            nvgFontSize(vg, 16);
            nvgFontFaceId(vg, font->handle);
            nvgTextLetterSpacing(vg, 0.5);
        }

        std::stringstream to_display;
        float display_val = 12.0;
        if (module)
            display_val = module->display_val[display_num];

        to_display = format4display(display_val);

        Vec textPos = Vec(8.0f, 33.0f);
        NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
        nvgFillColor(vg, textColor);
        nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);

        nvgFontSize(vg, 8);
        textPos = Vec(1.0f, (display_val<0?28.0f:32.0f));
        nvgText(vg, textPos.x, textPos.y, (display_val<0?"-":"+"), NULL);
    }
};

struct RangeWidget: ModuleWidget {
    RangeWidget(Range *module);
};

RangeWidget::RangeWidget(Range *module) {
		setModule(module);
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Range.svg")));
        addChild(panel);
    }

    addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

    RangeDisplayWidget *display;
    int j = 0;
    for (int i = 0; i < 6; i++) {
        display = new RangeDisplayWidget;
        display->box.pos = Vec(23+i%2*60, 60+j*100);
        display->box.size = Vec(50, 40);
        display->module = module;
        display->display_num = i;
        addChild(display);
        if (i < 4)
            addParam(createParam<RoundBlackKnob>(Vec(28+i%2*60, 105+j*100), module, Range::CH1_PARAM+i));
        j += i%2;
    }

    addInput(createInput<PJ301MPort>(Vec(35, 323), module, Range::CH1_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(95, 323), module, Range::CH1_OUTPUT));

}

Model *modelRange = createModel<Range, RangeWidget>("Range");
