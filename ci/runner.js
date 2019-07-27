"use strict";

const process = require('process');
const cp = require('child_process');

class Runner {
    constructor(name, cmd, args, env) {
        this.name = name;
        this.cmd = cmd;
		if (args == undefined)
			args = [];
        this.args = args;
		if (env == undefined)
			env = process.env;
		this.env = env;
    }
    
	run() {
        let self = this;
        return new Promise(function(resolve, reject) {
			console.log(self.cmd, self.args);
            self.proc = cp.spawn(
                self.cmd, self.args, {
                    windowsVerbatimArguments: true,
                    windowsHide: true,
					env: self.env,
                }
            );
            self.proc.stdout.on('data', (data) => {
                process.stdout.write(`[${self.name}:Out] ${data}`);
            });
            self.proc.stderr.on('data', (data) => {
                process.stderr.write(`[${self.name}:Err] ${data}`);
            });
            self.proc.on('exit', (code, signal) => {
                if (code != 0) {
                    reject(code);
                    return;
                }
                resolve(code);
                return;
            });
        });
	}
	
    execute() {
		return this.run();
    }
}

module.exports = Runner;