"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// lib/trigger/build-trigger.complete.ts
var build_trigger_complete_exports = {};
__export(build_trigger_complete_exports, {
  handler: () => handler
});
module.exports = __toCommonJS(build_trigger_complete_exports);
var import_aws_sdk = require("aws-sdk");
var codebuild = new import_aws_sdk.CodeBuild();
async function handler(event) {
  console.log(JSON.stringify(event));
  if (event.RequestType === "Delete")
    return { IsComplete: true };
  const { buildId } = event;
  const response = await codebuild.batchGetBuilds({ ids: [buildId] }).promise();
  const [{ buildStatus }] = response.builds;
  if (buildStatus === "SUCCEEDED")
    return { IsComplete: true };
  if (buildStatus === "IN_PROGRESS")
    return { IsComplete: false };
  throw new Error(`Something went wrong with the build id: ${buildId}, with status: ${buildStatus}`);
}
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  handler
});
