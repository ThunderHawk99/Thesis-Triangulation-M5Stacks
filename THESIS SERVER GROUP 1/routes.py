from flask import Blueprint, render_template, jsonify, request
from tag_processing import compute_plot_data, update_tag_distances

main = Blueprint('main', __name__)

@main.route('/')
def index():
    return render_template('index.html')

@main.route('/data')
def data():
    return jsonify(compute_plot_data())

@main.route('/update', methods=['POST'])
def update_measurements():
    content = request.json
    result = update_tag_distances(content)
    return jsonify(result)
