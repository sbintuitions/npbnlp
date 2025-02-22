#include"usbd_l.h"
#include"usbd_w.h"

using namespace std;
using namespace npbnlp;

bd_wrap::bd_wrap():_d(nullptr) {
}

bd_wrap::~bd_wrap() {
}

bool bd_wrap::create(int n, sequence_type type, smoothing s) {
	switch (type) {
		case sequence_type::word:
			_d = shared_ptr<usbd>(new usbd_w(n, s));
			return true;
		case sequence_type::letter:
			_d = shared_ptr<usbd>(new usbd_l(n, s));
			return true;
		default:
			return false;
	}
}

void bd_wrap::init(cio& f, int iter) {
	if (_d)
		_d->init(f, iter);
}

void bd_wrap::pretrain(io& f, int iter) {
	if (_d)
		_d->pretrain(f, iter);
}

void bd_wrap::set_punc(unsigned int c) {
	if (_d)
		_d->set_punc(c);
}

void bd_wrap::set_prior(double g, double c, double p) {
	if (_d)
		_d->set_prior(g, c, p);
}

void bd_wrap::set_general_prior(double p) {
	if (_d)
		_d->set_general_prior(p);
}

void bd_wrap::set_cr_prior(double p) {
	if (_d)
		_d->set_cr_prior(p);
}

void bd_wrap::set_punc_prior(double p) {
	if (_d)
		_d->set_punc_prior(p);
}

void bd_wrap::set_hyper_general(double a, double b) {
	if (_d)
		_d->set_hyper_general(a, b);
}

void bd_wrap::set_hyper_cr(double c, double d) { 
	if (_d)
		_d->set_hyper_cr(c, d);
}

void bd_wrap::set_hyper_punc(double e, double f) {
	if (_d)
		_d->set_hyper_punc(e, f);
}

void bd_wrap::add(io& f, vector<int>& head) {
	if (_d)
		_d->add(f, head);
}

void bd_wrap::remove(io& f, vector<int>& head) {
	if (_d)
		_d->remove(f, head);
}

void bd_wrap::estimate_lm_hyper(int iter) {
	if (_d)
		_d->estimate_lm_hyper(iter);
}

void bd_wrap::estimate_prior(cio& c, vector<vector<int> >& bd) {
	if (_d)
		_d->estimate_prior(c, bd);
}

void bd_wrap::eval(cio& c, cio& t, vector<double>& score) {
	if (_d)
		_d->eval(c, t, score);
}

void bd_wrap::save(const char *file) {
	if (_d)
		_d->save(file);
}

void bd_wrap::load(const char *file) {
	if (_d)
		_d->load(file);
}

void bd_wrap::sample(io& f, vector<int>& b) {
	if (_d)
		_d->sample(f, b);
}

void bd_wrap::parse(io& f, vector<int>& b) {
	if (_d)
		_d->parse(f, b);
}
