#include <iostream>

#include "Gui/CvGui.hpp"

using namespace CvGui;

// ------------------  M a i n ---------------------
int main() {
	// ------ Create GUI ------
	Gui<AddPolicy::Col> gui;
	auto interface = gui.createInterface("Interface_0");
	
	auto label 		= std::make_shared<Label>("List of Widgets: ");
	auto trackbar 	= std::make_shared<TrackBar>("Move Me: ");
	auto button 	= std::make_shared<PushButton>("Click Me !", Widget::Size(150, 50));
	auto checkBox 	= std::make_shared<CheckBox>("Check Me: ");
	auto spacer 	= std::make_shared<Spacer>(Widget::Size(50, 250));
	
	interface->add(spacer);
	interface->add(label, trackbar, button, checkBox);
	interface->add(spacer);
	
	gui.add(interface);
	
	// Attach events
	trackbar->listen(TrackBar::onValueChanged, [=](void*, void*) {
		std::cout << "TrackBar: " << trackbar->getValue() << std::endl;
	});
	button->listen(PushButton::onClick, [=](void*, void*) {
		std::cout << "Button clicked" << std::endl;
	});
	checkBox->listen(CheckBox::onValueChanged, [=](void*, void*) {
		std::cout << "CheckBox: " << checkBox->getChecked() << std::endl;
	});

	// ------ Loop it ------	
	gui.show();
	while(gui.wait(30) != gui.VK_ESCAPE) {
		// Do things
	}
	
	return 0;
}