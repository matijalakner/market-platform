#pragma once

#include"market/code/types.hpp"

namespace market {

    	class Market;
    	class TraderRegistry;
	class Settlement;
	class FundamentalValue;

	class Agent {
		public:
        	    	virtual ~Agent() = default;
            		virtual void step(
               		 	Timestamp timestamp,
                		Market& market,
            			TraderRegistry& traders,
	    			Settlement& settlement,
				FundamentalValue& fundamental_value
			) = 0;
    	};
}
