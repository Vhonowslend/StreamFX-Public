"use strict";

const process = require('process');
const runner = require('./runner.js');

function runRunners(runnerArray, name) {
	return new Promise(async (resolve, reject) => {
		let local = runnerArray.reverse();
		while (local.length > 0) {	
			let task = local.pop();
			let work = new runner(name, task[0], task[1], task[2]);
			await work.run();
		}
		resolve(0);
	});
}

let env = process.env;
let steps = [];

if ((process.env.CMAKE_GENERATOR_64 !== undefined) && (process.env.CMAKE_GENERATOR_64 !== "")) {
	steps.push(
		[ 'cmake', [
			'--build', 'build/64',
			'--config', 'RelWithDebInfo',
			'--target', 'PACKAGE_7Z'
		], env ]
	);
	steps.push(
		[ 'cmake', [
			'--build', 'build/64',
			'--config', 'RelWithDebInfo',
			'--target', 'PACKAGE_ZIP'
		], env ]
	);
} else if ((process.env.CMAKE_GENERATOR_32 !== undefined) && (process.env.CMAKE_GENERATOR_32 !== "")) {
	steps.push(
		[ 'cmake', [
			'--build', 'build/32',
			'--config', 'RelWithDebInfo',
			'--target', 'PACKAGE_7Z'
		], env ]
	);
	steps.push(
		[ 'cmake', [
			'--build', 'build/32',
			'--config', 'RelWithDebInfo',
			'--target', 'PACKAGE_ZIP'
		], env ]
	);
}

let promises = [];
promises.push(runRunners(steps, "32-Bit"));
Promise.all(promises).then(
	res => {
		process.exit(0);
	},
	err => {
		console.log(err);
		process.exit(1);
	}
)
