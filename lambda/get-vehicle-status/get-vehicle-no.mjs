import { ComputerVisionClient } from "@azure/cognitiveservices-computervision";
import { CognitiveServicesCredentials } from "@azure/ms-rest-azure-js";

export const getVehicleNo = async (imageUrl) => {
    const computerVisionKey = process.env.COMPUTER_VISION_KEY;
    const computerVisionEndPoint = process.env.COMPUTER_VISION_ENDPOINT;

    try {
        const cognitiveServiceCredentials = new CognitiveServicesCredentials(computerVisionKey);
        const computervisionClient = new ComputerVisionClient(cognitiveServiceCredentials, computerVisionEndPoint);

        // Call the OCR method on the image
        const readResponse = await computervisionClient.read(imageUrl, { raw: true });

        // Get the operation location URL from the response
        const operationLocation = readResponse.operationLocation;
        const operationId = operationLocation.split('/').pop(); // Extract the operation ID

        // Poll for the result until it is completed
        let readResult;
        let vehicleNo =  null;
        while (true) {
            readResult = await computervisionClient.getReadResult(operationId);

            if (readResult.status !== 'notStarted' && readResult.status !== 'running') {
                break;
            }

            console.log('Waiting for the result...');
            await new Promise(resolve => setTimeout(resolve, 1000));  // Sleep for 1 second
        }

        // Check if the OCR operation succeeded and print detected text with license plate detection
        if (readResult.status === 'succeeded') {
            readResult.analyzeResult.readResults.forEach(textResult => {
                textResult.lines.forEach(line => {
                    // Check if the line contains a license plate number (digits and letters)
                    if (/\d/.test(line.text) && /[a-zA-Z]/.test(line.text)) {
                        console.log('License plate number:', line.text);
                        vehicleNo = line.text;
                    }
                });
            });
        } else {
            console.error('OCR operation failed or timed out');
        }
        return vehicleNo;
    } catch (error) {
        console.error('Error during OCR process:', error);
    }
};