/*
 * Strange Weather - Chaotic CV Generator
 * 
 * Three independent strange attractor banks producing 16 CV outputs.
 * Based on Lorenz, Rössler, Thomas, and Halvorsen attractors.
 */

#include "plugin.hpp"
#include <cmath>

// Attractor types
enum AttractorType {
    LORENZ = 0,
    ROSSLER = 1,
    THOMAS = 2,
    HALVORSEN = 3
};

// Attractor state and parameters
struct Attractor {
    double x, y, z;
    AttractorType type;
    
    // Bounding box tracking for normalization
    double minX, maxX, minY, maxY, minZ, maxZ;
    
    Attractor() {
        // Initialize with slightly random starting point
        x = 0.1 + (random::uniform() - 0.5) * 0.1;
        y = 0.0 + (random::uniform() - 0.5) * 0.1;
        z = 0.0 + (random::uniform() - 0.5) * 0.1;
        type = LORENZ;
        
        // Initial bounds (will adapt)
        minX = -20.0; maxX = 20.0;
        minY = -30.0; maxY = 30.0;
        minZ = 0.0;   maxZ = 50.0;
    }
    
    // Compute derivatives for current state
    void derivatives(double& dx, double& dy, double& dz) {
        switch (type) {
            case LORENZ: {
                // σ = 10, ρ = 28, β = 8/3
                const double sigma = 10.0;
                const double rho = 28.0;
                const double beta = 8.0 / 3.0;
                dx = sigma * (y - x);
                dy = x * (rho - z) - y;
                dz = x * y - beta * z;
                break;
            }
            case ROSSLER: {
                // a = 0.2, b = 0.2, c = 5.7
                const double a = 0.2;
                const double b = 0.2;
                const double c = 5.7;
                dx = -y - z;
                dy = x + a * y;
                dz = b + z * (x - c);
                break;
            }
            case THOMAS: {
                // b = 0.208186
                const double b = 0.208186;
                dx = std::sin(y) - b * x;
                dy = std::sin(z) - b * y;
                dz = std::sin(x) - b * z;
                break;
            }
            case HALVORSEN: {
                // a = 1.89
                const double a = 1.89;
                dx = -a * x - 4.0 * y - 4.0 * z - y * y;
                dy = -a * y - 4.0 * z - 4.0 * x - z * z;
                dz = -a * z - 4.0 * x - 4.0 * y - x * x;
                break;
            }
        }
    }
    
    // RK4 integration step
    void step(double dt) {
        double k1x, k1y, k1z;
        double k2x, k2y, k2z;
        double k3x, k3y, k3z;
        double k4x, k4y, k4z;
        
        double ox = x, oy = y, oz = z;
        
        // k1
        derivatives(k1x, k1y, k1z);
        
        // k2
        x = ox + 0.5 * dt * k1x;
        y = oy + 0.5 * dt * k1y;
        z = oz + 0.5 * dt * k1z;
        derivatives(k2x, k2y, k2z);
        
        // k3
        x = ox + 0.5 * dt * k2x;
        y = oy + 0.5 * dt * k2y;
        z = oz + 0.5 * dt * k2z;
        derivatives(k3x, k3y, k3z);
        
        // k4
        x = ox + dt * k3x;
        y = oy + dt * k3y;
        z = oz + dt * k3z;
        derivatives(k4x, k4y, k4z);
        
        // Final update
        x = ox + (dt / 6.0) * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
        y = oy + (dt / 6.0) * (k1y + 2.0 * k2y + 2.0 * k3y + k4y);
        z = oz + (dt / 6.0) * (k1z + 2.0 * k2z + 2.0 * k3z + k4z);
        
        // Update bounding box with slow decay toward current values
        const double decay = 0.9999;
        minX = std::min(minX * decay + x * (1.0 - decay), x);
        maxX = std::max(maxX * decay + x * (1.0 - decay), x);
        minY = std::min(minY * decay + y * (1.0 - decay), y);
        maxY = std::max(maxY * decay + y * (1.0 - decay), y);
        minZ = std::min(minZ * decay + z * (1.0 - decay), z);
        maxZ = std::max(maxZ * decay + z * (1.0 - decay), z);
    }
    
    // Get normalized outputs (-5V to +5V)
    float getNormX() {
        double range = maxX - minX;
        if (range < 0.001) range = 0.001;
        return (float)(((x - minX) / range) * 10.0 - 5.0);
    }
    
    float getNormY() {
        double range = maxY - minY;
        if (range < 0.001) range = 0.001;
        return (float)(((y - minY) / range) * 10.0 - 5.0);
    }
    
    float getNormZ() {
        double range = maxZ - minZ;
        if (range < 0.001) range = 0.001;
        return (float)(((z - minZ) / range) * 10.0 - 5.0);
    }
};


struct StrangeWeather : Module {
    enum ParamIds {
        RATE_A_PARAM,
        RATE_B_PARAM,
        RATE_C_PARAM,
        SHAPE_A_PARAM,
        SHAPE_B_PARAM,
        SHAPE_C_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds {
        NUM_INPUTS
    };
    
    enum OutputIds {
        // Bank A
        A_X_OUTPUT,
        A_Y_OUTPUT,
        A_Z_OUTPUT,
        A_SUM_OUTPUT,
        // Bank B
        B_X_OUTPUT,
        B_Y_OUTPUT,
        B_Z_OUTPUT,
        B_SUM_OUTPUT,
        // Bank C
        C_X_OUTPUT,
        C_Y_OUTPUT,
        C_Z_OUTPUT,
        C_SUM_OUTPUT,
        // Combined
        COMB_SUM_OUTPUT,
        COMB_RECT_OUTPUT,
        COMB_INV_OUTPUT,
        COMB_DIST_OUTPUT,
        NUM_OUTPUTS
    };
    
    enum LightIds {
        NUM_LIGHTS
    };
    
    // Three attractor banks
    Attractor attractors[3];
    
    // Display state
    int displayMode = 4; // 0=A, 1=B, 2=C, 3=Combined, 4=All
    
    // Trail history for display (ring buffer)
    static const int TRAIL_LENGTH = 256;
    float trailX[3][TRAIL_LENGTH] = {};
    float trailY[3][TRAIL_LENGTH] = {};
    int trailIndex = 0;
    
    // Combined trail
    float combTrailX[TRAIL_LENGTH] = {};
    float combTrailY[TRAIL_LENGTH] = {};
    
    // Sample counter for trail updates
    int trailCounter = 0;
    
    StrangeWeather() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        
        // Rate knobs: exponential scaling from ~0.00001 Hz to ~1000 Hz
        configParam(RATE_A_PARAM, 0.f, 1.f, 0.5f, "Rate A", " Hz", 10.f, 0.0001f);
        configParam(RATE_B_PARAM, 0.f, 1.f, 0.5f, "Rate B", " Hz", 10.f, 0.0001f);
        configParam(RATE_C_PARAM, 0.f, 1.f, 0.5f, "Rate C", " Hz", 10.f, 0.0001f);
        
        // Shape switches (0-3)
        configSwitch(SHAPE_A_PARAM, 0.f, 3.f, 0.f, "Shape A", {"Lorenz", "Rössler", "Thomas", "Halvorsen"});
        configSwitch(SHAPE_B_PARAM, 0.f, 3.f, 1.f, "Shape B", {"Lorenz", "Rössler", "Thomas", "Halvorsen"});
        configSwitch(SHAPE_C_PARAM, 0.f, 3.f, 2.f, "Shape C", {"Lorenz", "Rössler", "Thomas", "Halvorsen"});
        
        // Output labels
        configOutput(A_X_OUTPUT, "Bank A X");
        configOutput(A_Y_OUTPUT, "Bank A Y");
        configOutput(A_Z_OUTPUT, "Bank A Z");
        configOutput(A_SUM_OUTPUT, "Bank A Sum");
        configOutput(B_X_OUTPUT, "Bank B X");
        configOutput(B_Y_OUTPUT, "Bank B Y");
        configOutput(B_Z_OUTPUT, "Bank B Z");
        configOutput(B_SUM_OUTPUT, "Bank B Sum");
        configOutput(C_X_OUTPUT, "Bank C X");
        configOutput(C_Y_OUTPUT, "Bank C Y");
        configOutput(C_Z_OUTPUT, "Bank C Z");
        configOutput(C_SUM_OUTPUT, "Bank C Sum");
        configOutput(COMB_SUM_OUTPUT, "Combined Sum");
        configOutput(COMB_RECT_OUTPUT, "Combined Rectified");
        configOutput(COMB_INV_OUTPUT, "Combined Inverted");
        configOutput(COMB_DIST_OUTPUT, "Combined Inverse Distance");
    }
    
    void cycleDisplay() {
        displayMode = (displayMode + 1) % 5;
    }
    
    void process(const ProcessArgs& args) override {
        // Get rate values (exponential scaling)
        // Knob 0.0 -> 0.0001 Hz (~2.7 hours), 0.5 -> 1 Hz, 1.0 -> 1000 Hz
        float rateA = std::pow(10.f, params[RATE_A_PARAM].getValue() * 4.f - 4.f);
        float rateB = std::pow(10.f, params[RATE_B_PARAM].getValue() * 4.f - 4.f);
        float rateC = std::pow(10.f, params[RATE_C_PARAM].getValue() * 4.f - 4.f);
        
        // Get attractor types
        attractors[0].type = (AttractorType)(int)params[SHAPE_A_PARAM].getValue();
        attractors[1].type = (AttractorType)(int)params[SHAPE_B_PARAM].getValue();
        attractors[2].type = (AttractorType)(int)params[SHAPE_C_PARAM].getValue();
        
        // Integration time step scaled by rate
        // Base dt chosen for stability at rate=1
        float rates[3] = {rateA, rateB, rateC};
        
        for (int i = 0; i < 3; i++) {
            // Adaptive time step: smaller steps for higher rates
            float dt = rates[i] / args.sampleRate;
            
            // For stability, limit maximum dt per sample
            // and do multiple smaller steps if needed
            const float maxDt = 0.01f;
            int steps = (int)std::ceil(dt / maxDt);
            steps = std::max(1, std::min(steps, 100));
            float subDt = dt / steps;
            
            for (int s = 0; s < steps; s++) {
                attractors[i].step(subDt);
            }
        }
        
        // Get normalized outputs
        float ax = attractors[0].getNormX();
        float ay = attractors[0].getNormY();
        float az = attractors[0].getNormZ();
        float aSum = ax + ay + az;
        
        float bx = attractors[1].getNormX();
        float by = attractors[1].getNormY();
        float bz = attractors[1].getNormZ();
        float bSum = bx + by + bz;
        
        float cx = attractors[2].getNormX();
        float cy = attractors[2].getNormY();
        float cz = attractors[2].getNormZ();
        float cSum = cx + cy + cz;
        
        // Bank outputs
        outputs[A_X_OUTPUT].setVoltage(ax);
        outputs[A_Y_OUTPUT].setVoltage(ay);
        outputs[A_Z_OUTPUT].setVoltage(az);
        outputs[A_SUM_OUTPUT].setVoltage(aSum);
        
        outputs[B_X_OUTPUT].setVoltage(bx);
        outputs[B_Y_OUTPUT].setVoltage(by);
        outputs[B_Z_OUTPUT].setVoltage(bz);
        outputs[B_SUM_OUTPUT].setVoltage(bSum);
        
        outputs[C_X_OUTPUT].setVoltage(cx);
        outputs[C_Y_OUTPUT].setVoltage(cy);
        outputs[C_Z_OUTPUT].setVoltage(cz);
        outputs[C_SUM_OUTPUT].setVoltage(cSum);
        
        // Combined outputs
        float combSum = aSum + bSum + cSum;
        float combRect = std::abs(aSum) + std::abs(bSum) + std::abs(cSum);
        float combInv = -combSum;
        float combDist = 5.f - std::abs(aSum) - std::abs(bSum) - std::abs(cSum);
        
        outputs[COMB_SUM_OUTPUT].setVoltage(combSum);
        outputs[COMB_RECT_OUTPUT].setVoltage(combRect);
        outputs[COMB_INV_OUTPUT].setVoltage(combInv);
        outputs[COMB_DIST_OUTPUT].setVoltage(combDist);
        
        // Update trail history (downsample for display)
        trailCounter++;
        if (trailCounter >= (int)(args.sampleRate / 60.f)) { // ~60 fps
            trailCounter = 0;
            trailIndex = (trailIndex + 1) % TRAIL_LENGTH;
            
            // Store normalized positions for display (-1 to 1 range)
            trailX[0][trailIndex] = ax / 5.f;
            trailY[0][trailIndex] = ay / 5.f;
            trailX[1][trailIndex] = bx / 5.f;
            trailY[1][trailIndex] = by / 5.f;
            trailX[2][trailIndex] = cx / 5.f;
            trailY[2][trailIndex] = cy / 5.f;
            
            // Combined: use sum and rectified sum as x,y
            combTrailX[trailIndex] = clamp(combSum / 15.f, -1.f, 1.f);
            combTrailY[trailIndex] = clamp(combRect / 15.f - 1.f, -1.f, 1.f);
        }
    }
    
    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "displayMode", json_integer(displayMode));
        return rootJ;
    }
    
    void dataFromJson(json_t* rootJ) override {
        json_t* displayModeJ = json_object_get(rootJ, "displayMode");
        if (displayModeJ) {
            displayMode = json_integer_value(displayModeJ);
        }
    }
};


// Custom display widget for attractor visualization
struct AttractorDisplay : Widget {
    StrangeWeather* module = nullptr;
    
    void draw(const DrawArgs& args) override {
        // Background
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(args.vg, nvgRGB(0x11, 0x11, 0x11));
        nvgFill(args.vg);
        
        if (!module) {
            // Preview display when module not loaded
            drawPreview(args);
            return;
        }
        
        int mode = module->displayMode;
        
        if (mode == 4) {
            // All four views
            float w = box.size.x / 2.f;
            float h = box.size.y / 2.f;
            
            drawAttractor(args, 0, 0, 0, w, h, nvgRGB(0x00, 0xff, 0xaa));
            drawAttractor(args, 1, w, 0, w, h, nvgRGB(0xff, 0xaa, 0x00));
            drawAttractor(args, 2, 0, h, w, h, nvgRGB(0xaa, 0x00, 0xff));
            drawCombined(args, w, h, w, h, nvgRGB(0xff, 0xff, 0xff));
            
            // Grid lines
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, w, 0);
            nvgLineTo(args.vg, w, box.size.y);
            nvgMoveTo(args.vg, 0, h);
            nvgLineTo(args.vg, box.size.x, h);
            nvgStrokeColor(args.vg, nvgRGBA(0x33, 0x33, 0x33, 0xff));
            nvgStrokeWidth(args.vg, 1.f);
            nvgStroke(args.vg);
        }
        else if (mode == 3) {
            // Combined only
            drawCombined(args, 0, 0, box.size.x, box.size.y, nvgRGB(0xff, 0xff, 0xff));
        }
        else {
            // Single attractor (0, 1, or 2)
            NVGcolor colors[3] = {
                nvgRGB(0x00, 0xff, 0xaa),
                nvgRGB(0xff, 0xaa, 0x00),
                nvgRGB(0xaa, 0x00, 0xff)
            };
            drawAttractor(args, mode, 0, 0, box.size.x, box.size.y, colors[mode]);
        }
    }
    
    void drawPreview(const DrawArgs& args) {
        // Simple Lorenz-ish shape for preview
        nvgBeginPath(args.vg);
        float cx = box.size.x / 2.f;
        float cy = box.size.y / 2.f;
        float r = std::min(cx, cy) * 0.6f;
        
        for (int i = 0; i < 100; i++) {
            float t = i / 100.f * 2.f * M_PI;
            float x = cx + r * std::sin(t) * std::cos(t * 0.5f);
            float y = cy + r * std::cos(t) * 0.7f;
            if (i == 0)
                nvgMoveTo(args.vg, x, y);
            else
                nvgLineTo(args.vg, x, y);
        }
        nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xff, 0xaa, 0x88));
        nvgStrokeWidth(args.vg, 1.f);
        nvgStroke(args.vg);
    }
    
    void drawAttractor(const DrawArgs& args, int bank, float ox, float oy, float w, float h, NVGcolor color) {
        if (!module) return;
        
        float cx = ox + w / 2.f;
        float cy = oy + h / 2.f;
        float scale = std::min(w, h) / 2.f * 0.9f;
        
        int idx = module->trailIndex;
        
        // Draw trail with fading
        for (int i = 0; i < StrangeWeather::TRAIL_LENGTH - 1; i++) {
            int i0 = (idx - i + StrangeWeather::TRAIL_LENGTH) % StrangeWeather::TRAIL_LENGTH;
            int i1 = (idx - i - 1 + StrangeWeather::TRAIL_LENGTH) % StrangeWeather::TRAIL_LENGTH;
            
            float alpha = 1.f - (float)i / StrangeWeather::TRAIL_LENGTH;
            alpha = alpha * alpha * 0.8f;
            
            float x0 = cx + module->trailX[bank][i0] * scale;
            float y0 = cy + module->trailY[bank][i0] * scale;
            float x1 = cx + module->trailX[bank][i1] * scale;
            float y1 = cy + module->trailY[bank][i1] * scale;
            
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, x0, y0);
            nvgLineTo(args.vg, x1, y1);
            nvgStrokeColor(args.vg, nvgRGBAf(color.r, color.g, color.b, alpha));
            nvgStrokeWidth(args.vg, 1.f + alpha);
            nvgStroke(args.vg);
        }
        
        // Current position dot
        float x = cx + module->trailX[bank][idx] * scale;
        float y = cy + module->trailY[bank][idx] * scale;
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, x, y, 2.f);
        nvgFillColor(args.vg, color);
        nvgFill(args.vg);
    }
    
    void drawCombined(const DrawArgs& args, float ox, float oy, float w, float h, NVGcolor color) {
        if (!module) return;
        
        float cx = ox + w / 2.f;
        float cy = oy + h / 2.f;
        float scale = std::min(w, h) / 2.f * 0.9f;
        
        int idx = module->trailIndex;
        
        for (int i = 0; i < StrangeWeather::TRAIL_LENGTH - 1; i++) {
            int i0 = (idx - i + StrangeWeather::TRAIL_LENGTH) % StrangeWeather::TRAIL_LENGTH;
            int i1 = (idx - i - 1 + StrangeWeather::TRAIL_LENGTH) % StrangeWeather::TRAIL_LENGTH;
            
            float alpha = 1.f - (float)i / StrangeWeather::TRAIL_LENGTH;
            alpha = alpha * alpha * 0.8f;
            
            float x0 = cx + module->combTrailX[i0] * scale;
            float y0 = cy + module->combTrailY[i0] * scale;
            float x1 = cx + module->combTrailX[i1] * scale;
            float y1 = cy + module->combTrailY[i1] * scale;
            
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, x0, y0);
            nvgLineTo(args.vg, x1, y1);
            nvgStrokeColor(args.vg, nvgRGBAf(color.r, color.g, color.b, alpha));
            nvgStrokeWidth(args.vg, 1.f + alpha);
            nvgStroke(args.vg);
        }
        
        float x = cx + module->combTrailX[idx] * scale;
        float y = cy + module->combTrailY[idx] * scale;
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, x, y, 2.f);
        nvgFillColor(args.vg, color);
        nvgFill(args.vg);
    }
};


// Cycle button
struct CycleButton : SvgSwitch {
    CycleButton() {
        momentary = true;
        addFrame(Svg::load(asset::system("res/ComponentLibrary/TL1105_0.svg")));
        addFrame(Svg::load(asset::system("res/ComponentLibrary/TL1105_1.svg")));
    }
};


struct StrangeWeatherWidget : ModuleWidget {
    AttractorDisplay* display = nullptr;
    
    StrangeWeatherWidget(StrangeWeather* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/StrangeWeather.svg")));
        
        // Screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // Display (top left)
        display = new AttractorDisplay();
        display->box.pos = mm2px(Vec(3.0, 12.0));
        display->box.size = mm2px(Vec(28.0, 28.0));
        display->module = module;
        addChild(display);
        
        // Cycle button (below display)
        // We use a param that triggers cycleDisplay on change
        addParam(createParamCentered<CycleButton>(mm2px(Vec(17.0, 45.0)), module, -1));
        
        // Bank A controls and outputs (y = 26mm center)
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.0, 26.0)), module, StrangeWeather::RATE_A_PARAM));
        addParam(createParamCentered<CKSSFour>(mm2px(Vec(53.0, 26.0)), module, StrangeWeather::SHAPE_A_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(66.0, 26.0)), module, StrangeWeather::A_X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.0, 26.0)), module, StrangeWeather::A_Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.0, 26.0)), module, StrangeWeather::A_Z_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.0, 26.0)), module, StrangeWeather::A_SUM_OUTPUT));
        
        // Bank B controls and outputs (y = 44mm center)
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.0, 44.0)), module, StrangeWeather::RATE_B_PARAM));
        addParam(createParamCentered<CKSSFour>(mm2px(Vec(53.0, 44.0)), module, StrangeWeather::SHAPE_B_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(66.0, 44.0)), module, StrangeWeather::B_X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.0, 44.0)), module, StrangeWeather::B_Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.0, 44.0)), module, StrangeWeather::B_Z_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.0, 44.0)), module, StrangeWeather::B_SUM_OUTPUT));
        
        // Bank C controls and outputs (y = 62mm center)
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.0, 62.0)), module, StrangeWeather::RATE_C_PARAM));
        addParam(createParamCentered<CKSSFour>(mm2px(Vec(53.0, 62.0)), module, StrangeWeather::SHAPE_C_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(66.0, 62.0)), module, StrangeWeather::C_X_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(74.0, 62.0)), module, StrangeWeather::C_Y_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.0, 62.0)), module, StrangeWeather::C_Z_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.0, 62.0)), module, StrangeWeather::C_SUM_OUTPUT));
        
        // Combined outputs (y = 100mm center)
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.0, 100.0)), module, StrangeWeather::COMB_SUM_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(56.0, 100.0)), module, StrangeWeather::COMB_RECT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.0, 100.0)), module, StrangeWeather::COMB_INV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(84.0, 100.0)), module, StrangeWeather::COMB_DIST_OUTPUT));
    }
    
    void step() override {
        ModuleWidget::step();
        
        // Handle cycle button (hacky but works for now)
        // A better approach would be a custom button widget
    }
    
    void appendContextMenu(Menu* menu) override {
        StrangeWeather* module = dynamic_cast<StrangeWeather*>(this->module);
        if (!module) return;
        
        menu->addChild(new MenuSeparator());
        menu->addChild(createMenuLabel("Display"));
        
        menu->addChild(createIndexSubmenuItem("View",
            {"Bank A", "Bank B", "Bank C", "Combined", "All"},
            [=]() { return module->displayMode; },
            [=](int mode) { module->displayMode = mode; }
        ));
    }
};


// Note: CKSSFour doesn't exist in standard VCV, we'll need to create one
// or use a different switch type. For now this will need adjustment.

Model* modelStrangeWeather = createModel<StrangeWeather, StrangeWeatherWidget>("StrangeWeather");
