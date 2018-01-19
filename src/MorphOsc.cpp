#include "Jaffasplaffa.hpp"

/*
Font info for GUI
Used the font that used on start up, (sans serif I think)
For small text size 6
Don't remember big sizes
*/


struct MyModule : Module {
	enum ParamIds {
		PITCH_PARAM,
        BREAKPOINT_PARAM,
        RISE_PARAM,
        FALL_PARAM,
        SINTRI_PARAM,
        PHASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
        PITCH2_INPUT,
        PHASERESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SINE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	float phase = 0.0;
    float phase2 = 0.0;
	float blinkPhase = 0.0;
    float OLDRESET = 0.0;
    
    // New for the morph osc
    float SIGONEFIXED = 1.0; // 4
    float Breakpoint;
    float Rise;
    float Fall;
    float SinTri;
    float Phaseparam;

	MyModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void MyModule::step() {
	// Implement a simple sine oscillator
	float deltaTime = 1.0 / engineGetSampleRate(); // Delta time = Sample Rate. Recipr.

	// Compute the frequency from the pitch parameter and input
	float pitch = params[PITCH_PARAM].value;
	pitch += inputs[PITCH_INPUT].value + inputs[PITCH2_INPUT].value;
	pitch = clampf(pitch, -5.0, 5.0); // Set range in octaves. Was 5 before.
	float freq = 110.0 * powf(2.0, pitch); // base frequency. Changed from 440 to 110. Goes deeper like that.
    

    if(inputs[PHASERESET_INPUT].value && !OLDRESET)
    {
        phase = 0.0;
    }
    else
    {
        phase += freq * deltaTime;
    }
    OLDRESET =     inputs[PHASERESET_INPUT].value;
    
	// Accumulate the phase ???
	if (phase >= 1.0)
		phase -= 1.0;

  
    Breakpoint  = params[BREAKPOINT_PARAM].value;
    Rise        = params[RISE_PARAM].value;
    Fall        = params[FALL_PARAM].value;
    SinTri      = params[SINTRI_PARAM].value;
    Phaseparam  = params[PHASE_PARAM].value;


    
    float saw = phase;
    
    float PhasorDivBreakPoint       =   saw / Breakpoint;                                               // 1
    float PhasorlessthanBreakPoint  =   saw < Breakpoint;                                               // 2
    float PhasorgreatthanBreakPoint =   saw > Breakpoint;                                               // 3
    float SIGONEminusBreakpoint     =   SIGONEFIXED - Breakpoint;                                       // 4
    float PhasorMinusBreakPoint     =   saw - Breakpoint;                                               // 5
    
    float FiveDivWithFour           =   (PhasorMinusBreakPoint / SIGONEminusBreakpoint) * 0.5;          // 6/7
    float FourMinusFive             =   (SIGONEminusBreakpoint - PhasorMinusBreakPoint);                // 8
    float EightDivWithFour          =   FourMinusFive / SIGONEminusBreakpoint;                          // 9
    float NineMultiplyThree         =   EightDivWithFour * PhasorgreatthanBreakPoint;                   // 10
    float OneMultiplyTwo            =   PhasorDivBreakPoint * PhasorlessthanBreakPoint;                 // 11
    float ElevenPlusTen             =   OneMultiplyTwo + NineMultiplyThree;                             // 12
    float OneMultpilyHalfPlusHalf   =   (PhasorDivBreakPoint * 0.5) + 0.5;                              // 13
    float ThirtenMultiplyTwo        =   OneMultpilyHalfPlusHalf * PhasorlessthanBreakPoint;             // 14
    float SenvenMultiplyThree       =   FiveDivWithFour * PhasorgreatthanBreakPoint;                    // 15
    
    float FourteenPlusFifteen       =   ThirtenMultiplyTwo + SenvenMultiplyThree;                       // 16
    float Cosine                    =   (((((sinf(2 * M_PI * (FourteenPlusFifteen+Phaseparam))) * 0.5) + 0.5)* -1)+1); // 17 (inv. cos.)
    
    float SinetriMath               =   Cosine * ((SinTri * -1)+1);                                     // 18
    float SinetriMultiplyTwelve     =   SinTri * ElevenPlusTen;                                         // 19
    
    float EighteenPlusNineteen      =   SinetriMath + SinetriMultiplyTwelve;                            // 20
    
    float RiseMultiplyWithTwo       =   Rise * PhasorlessthanBreakPoint;                                // 21
    float FallMultiplyWithThree     =   Fall * PhasorgreatthanBreakPoint;                               // 22
    float TwentyOnePlusTwentyTwo    =   RiseMultiplyWithTwo + FallMultiplyWithThree;                    // 23
    float POWFUNCTION               =   powf(EighteenPlusNineteen,TwentyOnePlusTwentyTwo);              // 24
    
    outputs[SINE_OUTPUT].value = (10.0 * POWFUNCTION)-5;                                                // 2
    
}


MyModuleWidget::MyModuleWidget() {
	MyModule *module = new MyModule();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/MorphOsc.svg")));
		addChild(panel);
	}
    
    // Screws
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 *      RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH,      RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 *      RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    
    // Parameters
    addParam(createParam<Davies1900hBlackKnob>(Vec(28, 44),     module, MyModule::PITCH_PARAM,         -3.0, 3.0, 0.0));
    addParam(createParam<Davies1900hBlackKnob>(Vec(28, 92),     module, MyModule::BREAKPOINT_PARAM,    0.0001, 0.9999, 0.5));
    addParam(createParam<Davies1900hBlackKnob>(Vec(5, 134),     module, MyModule::RISE_PARAM,          0.0, 2.0, 1.0));
    addParam(createParam<Davies1900hBlackKnob>(Vec(50, 134),    module, MyModule::FALL_PARAM,          0.0, 2.0, 1.0));
    addParam(createParam<Davies1900hBlackKnob>(Vec(28, 178),    module, MyModule::SINTRI_PARAM,        0.0, 1.0, 1.0));
    addParam(createParam<Davies1900hBlackKnob>(Vec(28, 228),    module, MyModule::PHASE_PARAM,         0.0, 1.0, 0.0));


    // Inputs
    addInput(createInput<PJ301MPort>(Vec(3, 257), module,       MyModule::PITCH_INPUT));
    addInput(createInput<PJ301MPort>(Vec(62, 257), module,      MyModule::PITCH2_INPUT));
    addInput(createInput<PJ301MPort>(Vec(33, 287), module,      MyModule::PHASERESET_INPUT));

	addOutput(createOutput<PJ301MPort>(Vec(33, 320), module,    MyModule::SINE_OUTPUT));

}






