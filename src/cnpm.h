#pragma once

static std::string username="rimartin";
static std::string passphrase="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";

#define UNUSED(x) (void)(x)

static void replace(std::string& s, std::string ov, std::string nv){

    std::vector<size_t> positions;
    size_t previousOffset=s.length()-ov.length();
    size_t offset=s.length()-1;
    while((offset=s.rfind(ov, previousOffset))!=std::string::npos){
        positions.insert(positions.begin(), offset);
        previousOffset=offset-ov.length();
    }
    if(positions.empty())return;
    std::cout<<"positions "<<positions.size()<<std::endl;
    bool enlarging=nv.size()>ov.size();
    size_t delta=(enlarging) ? nv.size()-ov.size() : ov.size()-nv.size();
    size_t originalLength=s.size();
    size_t newLength=(enlarging) ? originalLength+positions.size()*delta : originalLength-positions.size()*delta;;
    if(enlarging){
        s.resize(newLength);
        size_t positionIndex=positions.size()-1;
        size_t marchingIndex=newLength-1;
        std::cout<<"positionIndex "<<positionIndex<<" "<<marchingIndex<<std::endl;
        for(size_t index=originalLength;(index--)>positions[0];){
            if(index<positions[positionIndex]){
                if(positionIndex>0)positionIndex--;
            }
            if(index>=positions[positionIndex] && index<positions[positionIndex]+ov.size()){
                for(size_t newValueIndex=nv.size();(newValueIndex--)>0;){
                    s.at(marchingIndex--)=nv.at(newValueIndex);
                }
                index-=ov.size();
            }
            else
                s.at(marchingIndex--)=s.at(index);
            std::cout<<s<<std::endl;
        }
    }
    if(!enlarging){
        size_t positionIndex=0;
        size_t marchingIndex=positions[0];
        for(size_t index=positions[0];index<originalLength;index++){
            if(index>positions[positionIndex]+ov.size()){
                if(positionIndex<positions.size()-1)positionIndex++;
            }
            if(index>=positions[positionIndex] && index<positions[positionIndex]+ov.size()){
                for(size_t newValueIndex=0;newValueIndex<nv.size();newValueIndex++){
                    s.at(marchingIndex++)=nv.at(newValueIndex);
                }
                index+=(ov.size()-1);
            }
            else
                s.at(marchingIndex++)=s.at(index);
        }
        s.resize(newLength);
    }
};