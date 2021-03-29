//
//  Dynamics.cpp
//  
//
//  Created by Roberto Moran Tovar on 12.03.21.
//
//Template to run a stochastic/deterministic simulation of the antigen and bcells dynamics.

#include "../lib/Immuno_functions.hpp"

#include <stdio.h>

//----------------------------------------------------------------------------------
using namespace std;

// Function to run de set of differential equations
void ODE(double const beta, double const nu, double const gamma, long long NT, double dT, int n_naive, vector<bcell*> & Naive, vector<vector < long double > > & Time_series_Bcells, vector < long double > & Time_series_Antigen, vector < int > & N_active_linages){
    double f = 0;
    double N_active_bcells = 0;
    int n_active_linages = 0;
    for(int t = 1; t< NT ; t++){ // for loop of time
        //Update the antigen
        Time_series_Antigen[t] = Time_series_Antigen[t-1] + (beta*Time_series_Antigen[t-1] - gamma*Time_series_Antigen[t-1]*N_active_bcells)*dT;
        if(Time_series_Antigen[t]<1){
            Time_series_Antigen[t] = 0;
        }
        N_active_bcells = 0;
        //Update Bcells
        for(int n = 0 ; n<n_naive ; n++){
            Time_series_Bcells[n][t] = Time_series_Bcells[n][t-1] + (nu*Time_series_Bcells[n][t-1])*dT*(Naive[n]->active);
            if(Naive[n]->active ==0){
                f = (Time_series_Antigen[t]/N_A)/((Time_series_Antigen[t]/N_A) + exp(20+Naive[n]->e));
                if(f>0.5){
                    Naive[n]->active = 1;
                    n_active_linages++;
                }
            }else{
                N_active_bcells = N_active_bcells + Time_series_Bcells[n][t];
            }
        }
        N_active_linages[t] = N_active_linages[t] + n_active_linages;
    }
}

//----------------------------------------------------------------------------------
int main(int argc, char* argv[]) //argv has 1:L 2:N , 3:T , 4:T0 , 5:beta , 6:nu , 7:gamma
{
    string Text_files_path = "../../../../../Dropbox/Research/Evolution_Immune_System/Text_files/Dynamics/Single_trajectory/";
    cout<<">Running simulation of the Bcells-Antigen dynamics ..."<< endl;
    clock_t t1,t2;
    t1=clock();
    //-----------------------------------------------------------------------------
    //Parameters:
    std::string beta_s (argv[5]);
    double beta = stod(beta_s);
    std::string nu_s (argv[6]);
    double nu = stod(nu_s);;
    std::string gamma_s (argv[7]);
    double gamma = stod(gamma_s);;
    int L  = atoi(argv[1]); //length of the sequence
    int L_alphabet (20); //length of the alphabet
    long long int N = atoi(argv[2]); // number of bcells
    int T = atoi(argv[3]); //number of days for the simulation
    int T0 = atoi(argv[4]); //initial number of days for the simulation
    double dT = 0.001; //time step
    long long int NT = (T-T0)/dT; //number of steps
    long long A_0 = exp(beta*T0);

    //------------Energy Matrix------------------------------------------------------
    vector < vector < double > > MJ;
    MJ.resize(L_alphabet);
    for (int k= 0; k<L_alphabet; k++)
    {
        (MJ[k]).resize(L_alphabet);
    };

    ifstream file("../Input_files/MJ2.txt");

    //------------ Alphabet ----------------------------------------------------------
    //Array with the Alphabet
    vector < string > Alphabet;
    Alphabet.resize(L_alphabet);
    ifstream file2("../Input_files/Alphabet.txt");
    cout << "The Alphabet is :";
    for (int k = 0; k < L_alphabet; k++) {

        file2 >> Alphabet[k];
        cout << Alphabet[k] ;
    
    }
    cout << "\n";
    for (unsigned int i = 0; i < L_alphabet; i++) {
        for (unsigned int j = 0; j < L_alphabet; j++) {
            file >> MJ[i][j];
        }
    }
    //------------- Antigen -------------------------------------------------------------
    //Array with the antigen
    string Antigen_aa;
    cout << "Insert the Aminoacid sequence of the antigen:\n";
    getline(cin, Antigen_aa);
    
    vector < int > Antigen;
    Antigen.resize(L);
    aa_to_positions(L, L_alphabet, Alphabet, Antigen, Antigen_aa);
    
    //---------Generating Bcells ---------------------------------------------------------
    //Array with Bcells
    vector < bcell > Bcells;
    Bcells.resize(N);
    generate_Bcells(N, L, L_alphabet, Bcells);
    //---------Choosing antigen-specific Bcells ---------------------------------------------------------
    //Array with Naive-specific Bcells
    vector < bcell* > Naive;
    int n_naive = 0;
    choose_naive_Bcells(N, L, L_alphabet, MJ, Antigen, Bcells, Naive, n_naive);
    
    //Matrix with the time series of the antigen-specific Bcells
    vector<vector < long double > > Time_series_Bcells;
    Time_series_Bcells.resize(n_naive);
    for(int n= 0; n<n_naive; n++)
    {
        Time_series_Bcells[n].resize(NT);
        Time_series_Bcells[n][0] = Naive[n]->cs;
    };
    
    
    //Array with time series of the antigen
    vector < long double > Time_series_Antigen;
    Time_series_Antigen.resize(NT);
    Time_series_Antigen[0] = A_0;
    
    //Array for time series of the number of active bcell linages
    vector <int> N_active_linages;
    N_active_linages.resize(NT);
    
    
    cout << n_naive << endl;
    
    //Output files
    ofstream fout (Text_files_path+"energies_L-"+std::to_string(L)+"_N-"+ std::to_string(N)+"_Antigen-"+Antigen_aa+".txt");
    ofstream fout_antigen (Text_files_path+"antigen_L-"+std::to_string(L)+"_N-"+ std::to_string(N)+"_Antigen-"+Antigen_aa+"_beta-"+beta_s+"_nu-"+nu_s+"_gamma-"+gamma_s+".txt");
    ofstream fout_bcells (Text_files_path+"bcells_L-"+std::to_string(L)+"_N-"+ std::to_string(N)+"_Antigen-"+Antigen_aa+"_beta-"+beta_s+"_nu-"+nu_s+"_gamma-"+gamma_s+".txt");
    ofstream fout_N_active_linages (Text_files_path+"N_active_linages_L-"+std::to_string(L)+"_N-"+ std::to_string(N)+"_Antigen-"+Antigen_aa+"_beta-"+beta_s+"_nu-"+nu_s+"_gamma-"+gamma_s+".txt");
    
    for (int n= 0; n<n_naive; n++)
    {
        fout << Naive[n]->e << endl;
    };
    
    // Run ODE
    ODE(beta, nu, gamma, NT, dT, n_naive, Naive, Time_series_Bcells, Time_series_Antigen, N_active_linages);
    
    //Print time series of antigen and bcells
    for(int t=0 ; t<NT; t++){
        fout_antigen << Time_series_Antigen[t] << endl;
        fout_N_active_linages << N_active_linages[t] << "\t";
        for (int n = 0 ; n<n_naive ; n++){
            fout_bcells << Time_series_Bcells[n][t] << "\t";
        }
        fout_bcells << endl;
    }
        
    
    fout.close();
    fout_antigen.close();
    fout_bcells.close();
    cout<< ">Simulation completed…"<< endl;
    t2= clock();
    cout<< "(Running time: "<< double(t2-t1)/CLOCKS_PER_SEC <<" seconds.)"<< endl;

    return 0;
}