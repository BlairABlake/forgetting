#include <cli/cli.h>
#include <cli/clilocalsession.h>
#include <cli/filehistorystorage.h>
#include <cli/loopscheduler.h>
#include <unordered_map>
#include <queue>
#include "forgetting_curve.h"
using namespace cli;

int main() {
    std::unordered_map<std::string, std::unique_ptr<ForgettingCurve>> table;
    std::ifstream f("history");
    std::string s;
    if(f.is_open()) {
        while(std::getline(f, s)) {
            std::list tokens = split(s);
            if (tokens.size() != 3) break;
            std::string key = tokens.front();
            tokens.pop_front();
            int s_ = std::stoi(tokens.front());
            tokens.pop_front();
            time_t t_ = std::stol(tokens.front());
            tokens.pop_front();
            table[key] = std::make_unique<ForgettingCurve>(t_, s_);
        } 
    }

    auto rootMenu = std::make_unique<Menu>("forgetting");
    rootMenu->Insert(
        "get", {"key"},
        [&table](std::ostream& out, const std::string& arg){
            if (table.find(arg) == table.end()) {
                out << "Key not found" << std::endl;
                return;
            }

            auto& fc = table[arg];
            out << fc->R() * 100 << "%" << std::endl;
        });
    rootMenu->Insert(
        "update", {"key"},
        [&table](std::ostream& out, const std::string& arg) {
            if (table.find(arg) == table.end()) {
                table[arg] = std::make_unique<ForgettingCurve>();
            } else {
                auto& fc = table[arg];
                fc->update();
            }

            out << "Successfully updated" << std::endl;
        }
    );
    rootMenu->Insert(
        "delete", {"key"},
        [&table](std::ostream& out, const std::string& arg) {
            if (table.find(arg) != table.end()) {
                table.erase(arg);
            } 

            out << "Successfully deleted" << std::endl;
        }
    );
    rootMenu->Insert(
        "list",
        [&table](std::ostream& out) {
            std::priority_queue<std::pair<float, std::string>> queue;
            for(auto iter = table.begin(); iter != table.end(); iter++) {
                queue.push(std::make_pair(iter->second->R(), iter->first));
            }

            while(!queue.empty()) {
                auto p = queue.top();
                queue.pop();
                out << p.second << ":\t" << p.first * 100.0f << "%" << std::endl;
            }
        }
    );
    Cli cli( std::move(rootMenu), std::make_unique<FileHistoryStorage>(".cli") );
    cli.ExitAction( [&table](auto& out){ 
        std::ofstream outfile("history");
        for(auto iter = table.cbegin(); iter != table.cend(); iter++) {
            outfile << iter->first << " " << iter->second->serialize() << std::endl;
        }
    });
    cli.StdExceptionHandler(
        [](std::ostream& out, const std::string& cmd, const std::exception& e)
        {
            out << "Exception caught in cli handler: "
                << e.what()
                << " handling command: "
                << cmd
                << ".\n";
        }
    );

    
    LoopScheduler scheduler;
    CliLocalTerminalSession localSession(cli, scheduler, std::cout, 200);

    localSession.ExitAction(
        [&scheduler](auto& out) // session exit action
        {
            scheduler.Stop();
        }
    );
    scheduler.Run();

    return 0;
}