from flask import Flask, request, jsonify, render_template
import joblib
from flask_cors import CORS
import numpy as np

app = Flask(__name__)
CORS(app)

# Load the model
model = joblib.load('model.pkl')

@app.route('/')
def man():
    return render_template('result.html') 

@app.route('/predict', methods=['GET','POST'])
def predict():
    if request.method == 'POST':
            age = data['age']
            gender = data['gender']
            smoking = data['smoking']
            alcohol = data['alcohol']
            caffeine = data['caffeine']
            activity = data['activity']
            height = data['height']
            weight = data['weight']

            # Update these lines based on the keys you're receiving from the frontend
            # bmi and bpm values might need to be handled differently if they're not sent directly
            # Example: bmi = data['bmi'] or bpm = data['bpm']
            bmi = data.get('bmi', 0)  
            bpm = data.get('bpm', 0)

            features = np.array([[age, gender, height, weight, bmi, bpm, smoking, caffeine, alcohol, activity]])

            prediction = model.predict(features)  # Run the model on the data

            # Map the prediction to the corresponding condition
            conditions = {
                1: "Normal",
                4: "Low-risk Bradycardia",
                5: "High-risk Bradycardia",
                2: "Low-risk Atrial Fibrillation",
                3: "High-risk Atrial Fibrillation"
            }
            condition = conditions.get(prediction[0], "Unknown condition")

            # Return the prediction to the frontend
    return render_template('result.html', prediction=condition)



if __name__ == '__main__':
    app.run(port=5000, debug=True)