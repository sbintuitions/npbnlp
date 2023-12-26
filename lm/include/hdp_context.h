#ifndef NPBNLP_HDP_CONTEXT_H
#define NPBNLP_HDP_CONTEXT_H

#include"context.h"
#include"rd.h"
#include"beta.h"
namespace npbnlp {
	class hdp_context : public context {
		public:
			hdp_context();
			hdp_context(int k, context *h);
			virtual ~hdp_context();
			bool add(int k, lm *m);
			bool remove(int k);
			context* find(int k);
			context* make(int k);
			//void save(FILE *file);
			//void load(FILE *file);
			void estimate_a(std::vector<double>& a, std::vector<double>& b, lm *m);
		private:
			using children = std::unordered_map<int, std::shared_ptr<hdp_context> >;
			bool _crp_add(int k, lm *m);
			//bool _crp_remove(int k);
			std::shared_ptr<children> _child;
	};
}

#endif
