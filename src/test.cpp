#include <TMB.hpp>

// Symmetric unbiased hyperbolic distribution in 1D, parametrized by shape parameter
// zeta>0 and scale parameter delta>0. If zeta is near 0, this is approximately a Gaussian
// with variance delta^2/zeta. If zeta is large, this is approximately a two-sided
// exponential with rate zeta/delta.
template<class Type> Type dhyperbolic(Type x,Type delta, Type zeta,int give_log=0)
{
  Type ld = -zeta*sqrt(pow(x/delta,2) + 1.0); // Log unnormalized density
  Type lc = log(2*delta)  + log(besselK(zeta,Type(1.0)));

  if(!give_log) return exp(ld - lc) ;
  else return ld - lc;
}

template<class Type>
Type objective_function<Type>::operator() ()
{
  DATA_VECTOR(nTests);
  DATA_VECTOR(nPos);
  DATA_INTEGER(modelswitch);

  PARAMETER_VECTOR(logI);
  PARAMETER_VECTOR(logr);
  // PARAMETER_VECTOR(resI);
  // PARAMETER_VECTOR(resr);

  PARAMETER(beta);
  PARAMETER(logIsigma);    Type Isigma    = exp(logIsigma);
  PARAMETER(logIzeta);     Type Izeta     = exp(logIzeta);
  // PARAMETER(logIsigmaRes); Type IsigmaRes = exp(logIsigmaRes);
  PARAMETER(logrsigma);    Type rsigma    = exp(logrsigma);
  // PARAMETER(logrsigmares); Type rsigmares = exp(logrsigmares);
  PARAMETER(logrzeta);     Type rzeta     = exp(logrzeta);

  int nT = nTests.size();

  vector<Type> Epos(nT);

  Type ans = 0;

  for(int i=0; i<nT ; ++i)
    {
      Epos(i) = exp(logI(i)+beta*log(nTests(i))); // +resI(i));
      // ans -= dnorm(resI(i),Type(0),IsigmaRes,1);
      // ans -= dnorm(log(nPos(i)),log(Epos(i)),1/sqrt(nPos(i)),1);
      ans -= dpois(nPos(i),Epos(i),1);
    }

  for(int i = 1; i<nT ; ++i)
    {
      switch(modelswitch)
	{
	case 1:
	  ans -= dhyperbolic(logI(i)-logI(i-1)-logr(i-1),Isigma,Izeta,1); // -resr(i-1)
	  break;
	case 2:
	  ans -= dnorm(logI(i)-logI(i-1)-logr(i-1),Type(0),Isigma,1);  // -resr(i-1)
	  ans += pow(logIzeta,2);
	  break;
	}
      // ans -= dnorm(resr(i-1),Type(0),rsigmares,1);
    }

  for(int i = 2; i<nT ; ++i)
    {
      switch(modelswitch)
	{
	case 1:
	  ans -= dhyperbolic(logr(i-1)-logr(i-2),rsigma,rzeta,1);
	  break;
	case 2:
	  ans -= dnorm(logr(i-1)-logr(i-2),Type(0),rsigma,1);
	  ans += pow(logrzeta,2);
	  break;
	}
    }

  ADREPORT(Epos);

  return(ans);
}
