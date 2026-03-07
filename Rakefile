Dir.glob('lib/tasks/*.rake').each do |file|
  load(file)
end

task 'md2review' do
  sh "bundle exec md2review chapter01.md > chapter01.re"
  sh "bundle exec md2review chapter02.md > chapter02.re"
end

task default: [:md2review, :pdf]
