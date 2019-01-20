#ifndef NEURALPID_H
#define NEURALPID_H
#include <Akka.h>
#include <Machinelearning.h>

class NeuralPid : public Actor {
		vector<unsigned> _topology;
		Net _pidNet;
		vector<double> _inputVals, _targetVals, _resultVals;
		double _target;

	public:
		NeuralPid();
		~NeuralPid();
		void preStart();
		Receive& createReceive();

	private:
		void newInput(double measurement);
		void target(double target);
};

#endif // NEURALPID_H
